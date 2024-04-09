#pragma once

#include <functional>
#include <libyang-cpp/Context.hpp>
#include <libnetconf2-cpp/Enum.hpp>
#include <memory>
#include <optional>
#include <string>
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

using LogCb = std::function<void(const nc_session*, LogLevel, const char*)>;

void setLogLevel(LogLevel level);
void setLogCallback(const LogCb& callback);

class Session {
public:
    Session(struct nc_session* session);
    ~Session();
    static std::unique_ptr<Session> connectSocket(const std::string& path, std::optional<libyang::Context> ctx = std::nullopt);
    static std::unique_ptr<Session> connectFd(const int source, const int sink, std::optional<libyang::Context> ctx = std::nullopt);
    [[nodiscard]] std::vector<std::string> capabilities() const;
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
protected:
    struct nc_session* m_session;
};
}
}
