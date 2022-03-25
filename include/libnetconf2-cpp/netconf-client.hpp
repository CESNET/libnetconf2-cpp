#pragma once

#include <functional>
#include <libyang-cpp/Context.hpp>
#include <libnetconf2-cpp/Enum.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct nc_session;

namespace libyang {
class Context;
class DataNode;
}

namespace libnetconf {
namespace client {

class ReportedError : public std::runtime_error {
public:
    ReportedError(const std::string& what);
    ~ReportedError() override;
};

using KbdInteractiveCb = std::function<std::string(const std::string&, const std::string&, const std::string&, bool)>;
using LogCb = std::function<void(LogLevel, const char*)>;

void setLogLevel(LogLevel level);
void setLogCallback(const LogCb& callback);

class Session {
public:
    Session(struct nc_session* session);
    ~Session();
    static std::unique_ptr<Session> connectPubkey(const std::string& host, const uint16_t port, const std::string& user, const std::string& pubPath, const std::string& privPath);
    static std::unique_ptr<Session> connectKbdInteractive(const std::string& host, const uint16_t port, const std::string& user, const KbdInteractiveCb& callback);
    static std::unique_ptr<Session> connectSocket(const std::string& path);
    static std::unique_ptr<Session> connectFd(const int source, const int sink);
    [[nodiscard]] std::vector<std::string_view> capabilities() const;
    std::optional<libyang::DataNode> get(const std::optional<std::string>& filter = std::nullopt);
    std::optional<libyang::DataNode> getData(const NmdaDatastore datastore, const std::optional<std::string>& filter = std::nullopt);
    void editConfig(const Datastore datastore,
                    const EditDefaultOp defaultOperation,
                    const EditTestOpt testOption,
                    const EditErrorOpt errorOption,
                    const std::string& data);
    void editData(const NmdaDatastore datastore, const std::string& data);
    void copyConfigFromString(const Datastore target, const std::string& data);
    std::optional<libyang::DataNode> rpc_or_action(const std::string& xmlData);
    void copyConfig(const Datastore source, const Datastore destination);
    void commit();
    void discard();

    libyang::Context libyangContext();
    struct nc_session* session_internal(); // FIXME: remove me
protected:
    struct nc_session* m_session;
};
}
}
