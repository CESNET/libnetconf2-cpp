/*
 * Copyright (C) 2019 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 * Written by Jan Kundrát <jan.kundrat@cesnet.cz>
 *
*/

#include <cstring>
#include <libyang-cpp/Context.hpp>
#include <libyang-cpp/DataNode.hpp>
#include <libnetconf2-cpp/netconf-client.hpp>
#include <mutex>
extern "C" {
#include <nc_client.h>
}
#include <sstream>
#include "UniqueResource.hpp"
#include "utils.hpp"

namespace libnetconf {

namespace impl {

static client::LogCb logCallback;

static void logViaCallback(const nc_session* session, NC_VERB_LEVEL level, const char* message)
{
    logCallback(session, libnetconf::utils::toLogLevel(level), message);
}


/** @short Initialization of the libnetconf2 library client

Just a safe wrapper over nc_client_init and nc_client_destroy, really.
*/
class ClientInit {
    ClientInit()
    {
        nc_client_init();
    }

    ~ClientInit()
    {
        nc_client_destroy();
    }

public:
    static ClientInit& instance()
    {
        static ClientInit lib;
        return lib;
    }

    ClientInit(const ClientInit&) = delete;
    ClientInit(ClientInit&&) = delete;
    ClientInit& operator=(const ClientInit&) = delete;
    ClientInit& operator=(ClientInit&&) = delete;
};

auto guarded(nc_rpc* ptr)
{
    return std::unique_ptr<nc_rpc, decltype([](auto rpc) constexpr { nc_rpc_free(rpc); })>(ptr);
}

namespace {
const auto getData_path = "/ietf-netconf-nmda:get-data/data";
const auto get_path = "/ietf-netconf:get/data";
}

using managed_rpc = std::invoke_result_t<decltype(guarded), nc_rpc*>;

std::optional<libyang::DataNode> do_rpc(struct nc_session* session, managed_rpc&& rpc, const char* dataIdentifier)
{
    uint64_t msgid;
    NC_MSG_TYPE msgtype;

    msgtype = nc_send_rpc(session, rpc.get(), 1000, &msgid);
    if (msgtype == NC_MSG_ERROR) {
        throw std::runtime_error{"Failed to send RPC"};
    }
    if (msgtype == NC_MSG_WOULDBLOCK) {
        throw std::runtime_error{"Timeout sending an RPC"};
    }

    lyd_node* raw_reply;
    lyd_node* envp;
    while (true) {
        msgtype = nc_recv_reply(session, rpc.get(), msgid, 20000, &envp, &raw_reply);
        auto replyInfo = libyang::wrapRawNode(envp);

        switch (msgtype) {
        case NC_MSG_ERROR:
            throw std::runtime_error{"Failed to receive an RPC reply"};
        case NC_MSG_WOULDBLOCK:
            throw std::runtime_error{"Timed out waiting for RPC reply"};
        case NC_MSG_REPLY_ERR_MSGID:
            throw std::runtime_error{"Received a wrong reply -- msgid mismatch"};
        case NC_MSG_NOTIF:
            continue;
        default:
            if (!raw_reply) { // <ok> reply, or empty data node, or error
                std::string msg;
                for (const auto& child : replyInfo.child()->siblings()) {
                    if (child.path() == "/ietf-netconf:rpc-reply/rpc-error") {
                        for (const auto& error : child.childrenDfs()) {
                            // Note that error-message is opaque node, rpc-path is opaque node when server sends a response with path from unimplemented models
                            if (error.path() == "/ietf-netconf:rpc-reply/rpc-error/error-message") {
                                msg += "Error: ";
                                msg += error.asOpaque().value();
                                msg += "\n";
                            }

                            if (error.path() == "/ietf-netconf:rpc-reply/rpc-error/error-path") {
                                msg += "Path: ";
                                msg += error.isOpaque() ? error.asOpaque().value() : error.asTerm().valueStr();
                                msg += "\n";
                            }
                        }
                    }
                }

                if (!msg.empty()) {
                    throw client::ReportedError{msg};
                }

                return std::nullopt;
            }
            auto wrapped = libyang::wrapRawNode(raw_reply);

            // If we have a dataIdentifier, then we'll need to look for it.
            // Some operations don't have that, and then the result data are just the wrapped node.
            if (!dataIdentifier) {
                return wrapped;
            }

            auto anydataValue = wrapped.findPath(dataIdentifier, libyang::InputOutputNodes::Output)->asAny().releaseValue();

            // If there's no anydata value, then that means we get empty (but valid) data.
            if (!anydataValue) {
                return std::nullopt;
            }

            return std::get<libyang::DataNode>(*anydataValue);
        }
    }
    __builtin_unreachable();
}

void do_rpc_ok(struct nc_session* session, managed_rpc&& rpc)
{
    auto x = do_rpc(session, std::move(rpc), nullptr);
    if (x) {
        throw std::runtime_error{"Unexpected DATA reply"};
    }
}
}

namespace client {

void setLogLevel(LogLevel level)
{
    nc_verbosity(utils::toLogLevel(level));
}

void setLogCallback(const client::LogCb& callback)
{
    impl::logCallback = callback;
    nc_set_print_clb_session(callback ? impl::logViaCallback : NULL);
}

libyang::Context Session::libyangContext()
{
    return libyang::createUnmanagedContext(const_cast<ly_ctx*>(nc_session_get_ctx(m_session)), nullptr);
}

Session::Session(struct nc_session* session)
    : m_session(session)
{
    impl::ClientInit::instance();
}

Session::~Session()
{
    ::nc_session_free(m_session, nullptr);
}

std::unique_ptr<Session> Session::connectFd(const int source, const int sink, std::optional<libyang::Context> ctx)
{
    impl::ClientInit::instance();

    auto session = std::make_unique<Session>(nc_connect_inout(source, sink, ctx ? libyang::retrieveContext(*ctx) : nullptr));
    if (!session->m_session) {
        throw std::runtime_error{"nc_connect_inout failed"};
    }
    return session;
}

std::unique_ptr<Session> Session::connectSocket(const std::string& path, std::optional<libyang::Context> ctx)
{
    impl::ClientInit::instance();

    auto session = std::make_unique<Session>(nc_connect_unix(path.c_str(), ctx ? libyang::retrieveContext(*ctx) : nullptr));
    if (!session->m_session) {
        throw std::runtime_error{"nc_connect_unix failed"};
    }
    return session;
}

std::vector<std::string> Session::capabilities() const
{
    std::vector<std::string> res;
    auto caps = nc_session_get_cpblts(m_session);
    while (*caps) {
        res.emplace_back(*caps);
        ++caps;
    }
    return res;
}

std::optional<libyang::DataNode> Session::get(const std::optional<std::string>& filter)
{
    auto rpc = impl::guarded(nc_rpc_get(filter ? filter->c_str() : nullptr, NC_WD_ALL, NC_PARAMTYPE_CONST));
    if (!rpc) {
        throw std::runtime_error("Cannot create get RPC");
    }
    return impl::do_rpc(m_session, std::move(rpc), impl::get_path);
}

const char* datastoreToString(NmdaDatastore datastore)
{
    switch (datastore) {
    case NmdaDatastore::Startup:
        return "ietf-datastores:startup";
    case NmdaDatastore::Running:
        return "ietf-datastores:running";
    case NmdaDatastore::Candidate:
        return "ietf-datastores:candidate";
    case NmdaDatastore::Operational:
        return "ietf-datastores:operational";
    }
    __builtin_unreachable();
}

std::optional<libyang::DataNode> Session::getData(const NmdaDatastore datastore, const std::optional<std::string>& filter)
{
    auto rpc = impl::guarded(nc_rpc_getdata(datastoreToString(datastore), filter ? filter->c_str() : nullptr, nullptr, nullptr, 0, 0, 0, 0, NC_WD_ALL, NC_PARAMTYPE_CONST));
    if (!rpc) {
        throw std::runtime_error("Cannot create get RPC");
    }
    return impl::do_rpc(m_session, std::move(rpc), impl::getData_path);
}

void Session::editData(const NmdaDatastore datastore, const std::string& data)
{
    auto rpc = impl::guarded(nc_rpc_editdata(datastoreToString(datastore), NC_RPC_EDIT_DFLTOP_MERGE, data.c_str(), NC_PARAMTYPE_CONST));
    if (!rpc) {
        throw std::runtime_error("Cannot create get RPC");
    }
    return impl::do_rpc_ok(m_session, std::move(rpc));
}

void Session::editConfig(const Datastore datastore,
                         const EditDefaultOp defaultOperation,
                         const EditTestOpt testOption,
                         const EditErrorOpt errorOption,
                         const std::string& data)
{
    auto rpc = impl::guarded(
            nc_rpc_edit(
                utils::toDatastore(datastore),
                utils::toDefaultOp(defaultOperation),
                utils::toTestOpt(testOption),
                utils::toErrorOpt(errorOption),
                data.c_str(),
                NC_PARAMTYPE_CONST));
    if (!rpc) {
        throw std::runtime_error("Cannot create edit-config RPC");
    }
    impl::do_rpc_ok(m_session, std::move(rpc));
}

void Session::copyConfigFromString(const Datastore target, const std::string& data)
{
    auto rpc = impl::guarded(nc_rpc_copy(utils::toDatastore(target), nullptr, utils::toDatastore(target) /* yeah, cannot be 0... */, data.c_str(), NC_WD_UNKNOWN, NC_PARAMTYPE_CONST));
    if (!rpc) {
        throw std::runtime_error("Cannot create copy-config RPC");
    }
    impl::do_rpc_ok(m_session, std::move(rpc));
}

void Session::commit()
{
    auto rpc = impl::guarded(nc_rpc_commit(0, /* "Optional confirm timeout" how do you optional an uint32_t? */ 0, nullptr, nullptr, NC_PARAMTYPE_CONST));
    if (!rpc) {
        throw std::runtime_error("Cannot create commit RPC");
    }
    impl::do_rpc_ok(m_session, std::move(rpc));
}

void Session::discard()
{
    auto rpc = impl::guarded(nc_rpc_discard());
    if (!rpc) {
        throw std::runtime_error("Cannot create discard RPC");
    }
    impl::do_rpc_ok(m_session, std::move(rpc));
}

std::optional<libyang::DataNode> Session::rpc_or_action(const std::string& xmlData)
{
    auto rpc = impl::guarded(nc_rpc_act_generic_xml(xmlData.c_str(), NC_PARAMTYPE_CONST));
    if (!rpc) {
        throw std::runtime_error("Cannot create generic RPC");
    }

    return impl::do_rpc(m_session, std::move(rpc), nullptr);
}

void Session::copyConfig(const Datastore source, const Datastore destination)
{
    auto rpc = impl::guarded(nc_rpc_copy(utils::toDatastore(destination), nullptr, utils::toDatastore(source), nullptr, NC_WD_UNKNOWN, NC_PARAMTYPE_CONST));
    if (!rpc) {
        throw std::runtime_error("Cannot create copy-config RPC");
    }
    impl::do_rpc_ok(m_session, std::move(rpc));
}

ReportedError::ReportedError(const std::string& what)
    : std::runtime_error(what)
{
}

ReportedError::~ReportedError() = default;
}
}
