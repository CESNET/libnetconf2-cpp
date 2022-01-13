/*
 * Copyright (C) 2021 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/process.hpp>
#include <doctest/doctest.h>
#include <filesystem>
#include <libnetconf2-cpp/netconf-client.hpp>
#include <optional>
#include "test_vars.hpp"
#include "yanglib.hpp"

using namespace std::string_literals;

std::string escapeXMLchars(const std::string& input)
{
    std::ostringstream oss;
    for (const auto& ch : input) {

        switch (ch) {
        case '"':
            oss << "&quot;";
            break;
        case '\'':
            oss << "&apos;";
           break;
        case '<':
            oss << "&lt;";
            break;
        case '>':
            oss << "&gt;";
            break;
        case '&':
            oss << "&amp;";
            break;
        default:
            oss << ch;
        }
    }

    return oss.str();
}

void sendMsgWithSize(boost::process::opstream& processInput, const std::string& msg)
{
    processInput << "\n#" << msg.size() << "\n" << msg << "\n\n##\n";
    processInput.flush();
}

#define RPC_REPLY_START(MSGID) (R"(<rpc-reply xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id=")" + std::to_string(MSGID) + R"(">)"s)
#define RPC_REPLY_END R"(</rpc-reply>)"s

#define MODULE_MESSAGE_START(MSGID) RPC_REPLY_START(MSGID) + R"(<data xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring">)"s
#define MODULE_MESSAGE_END (R"(</data>)"s + RPC_REPLY_END)

// FIXME: This is copied from what the Netopeer2 server sends. Some capabilities were removed.
// I better make sure to check which of these are actually needed.
const auto serverHello = R"(
<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <capabilities>
        <capability>urn:ietf:params:netconf:base:1.0</capability>
        <capability>urn:ietf:params:netconf:base:1.1</capability>
        <capability>urn:ietf:params:netconf:capability:writable-running:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:candidate:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:confirmed-commit:1.1</capability>
        <capability>urn:ietf:params:netconf:capability:rollback-on-error:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:validate:1.1</capability>
        <capability>urn:ietf:params:netconf:capability:startup:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:xpath:1.0</capability>
        <capability>
            urn:ietf:params:netconf:capability:with-defaults:1.0?basic-mode=explicit&amp;also-supported=report-all,report-all-tagged,trim,explicit
        </capability>
        <capability>urn:ietf:params:netconf:capability:interleave:1.0</capability>
        <capability>urn:ietf:params:netconf:capability:url:1.0?scheme=scp,http,https,ftp,sftp,ftps,file</capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-yang-metadata?module=ietf-yang-metadata&amp;revision=2016-08-05
        </capability>
        <capability>urn:ietf:params:xml:ns:yang:1?module=yang&amp;revision=2021-04-07</capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-inet-types?module=ietf-inet-types&amp;revision=2013-07-15
        </capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-yang-types?module=ietf-yang-types&amp;revision=2013-07-15
        </capability>
        <capability>
            urn:ietf:params:netconf:capability:yang-library:1.1?revision=2019-01-04&amp;content-id=27
        </capability>
        <capability>
            urn:ietf:params:xml:ns:netconf:base:1.0?module=ietf-netconf&amp;revision=2013-09-29&amp;features=writable-running,candidate,confirmed-commit,rollback-on-error,validate,startup,url,xpath
        </capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-netconf-acm?module=ietf-netconf-acm&amp;revision=2018-02-14
        </capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults?module=ietf-netconf-with-defaults&amp;revision=2011-06-01
        </capability>
        <capability>
            urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring?module=ietf-netconf-monitoring&amp;revision=2010-10-04
        </capability>
    </capabilities>
    <session-id>1</session-id>
</hello>
]]>]]>
)";

std::string readFileToString(const std::filesystem::path& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
        throw std::invalid_argument("File '" + std::string(path) + "' does not exist.");

    std::istreambuf_iterator<char> begin(ifs), end;
    return {begin, end};
}

#define SEND_HELLO processInput << serverHello
#define LOAD_MODULE_STR(NAME_REV) readFileToString(TESTS_DIR "/modules/" NAME_REV ".yang")
#define SEND_RPC(MSGID, data) sendMsgWithSize(processInput, (RPC_REPLY_START(MSGID) + (data) + RPC_REPLY_END))
#define SEND_MODULE(MSGID, moduleName) sendMsgWithSize(processInput, MODULE_MESSAGE_START(MSGID) + escapeXMLchars(LOAD_MODULE_STR(moduleName)) + MODULE_MESSAGE_END)

void skipHello(boost::process::ipstream& processOutput)
{
    std::string msg;
    while (!msg.ends_with("]]>]]>")) {
        msg.push_back(char(processOutput.get()));
    }
}

#define SEND_OK(MSGID) sendMsgWithSize(processInput, (RPC_REPLY_START(MSGID) + (R"(<ok/>)") + RPC_REPLY_END))

void skipNetconfChunk(boost::process::ipstream& processOutput)
{
    REQUIRE(processOutput.get() == '\n');
    REQUIRE(processOutput.get() == '#');
    int size;
    processOutput >> size;
    processOutput.ignore(size + 1);
    REQUIRE(processOutput.get() == '\n');
    REQUIRE(processOutput.get() == '#');
    REQUIRE(processOutput.get() == '#');
    REQUIRE(processOutput.get() == '\n');
}

TEST_CASE("client")
{
    boost::process::ipstream processOutput;
    boost::process::opstream processInput;

    int curMsgId = 1;

    libnetconf::client::setLogLevel(NC_VERB_DEBUG);
    auto x = std::thread{[&processInput, &processOutput] {
        auto session = libnetconf::client::Session::connectFd(processInput.pipe().native_source(), processOutput.pipe().native_sink());

        // auto data = session->getData(libnetconf::NmdaDatastore::Running);
        // std::cerr << "data->printStr(libyang::DataFormat::JSON, libyang::PrintFlags::WithSiblings) = " << *data->printStr(libyang::DataFormat::JSON, libyang::PrintFlags::WithSiblings) << "\n";
    }};

    SEND_HELLO;
    skipHello(processOutput);
    SEND_MODULE(curMsgId++, "ietf-netconf@2013-09-29");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-netconf-acm@2018-02-14");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-yang-metadata@2016-08-05");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-yang-library@2019-01-04");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-datastores@2018-02-14");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-netconf-nmda@2019-01-07");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-origin@2018-02-14");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-netconf-with-defaults@2011-06-01");
    skipNetconfChunk(processOutput);
    SEND_RPC(curMsgId++, yangLib);
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-netconf-notifications@2012-02-06");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "nc-notifications@2008-07-14");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "notifications@2008-07-14");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-x509-cert-to-name@2014-12-10");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-crypto-types@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-keystore@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-truststore@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-tcp-common@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-ssh-server@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-ssh-common@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "iana-crypt-hash@2014-08-06");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-tls-server@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-tls-common@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-netconf-server@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-tcp-client@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-tcp-server@2019-07-02");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-interfaces@2018-02-20");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-ip@2018-02-22");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-network-instance@2019-01-21");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-subscribed-notifications@2019-09-09");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-restconf@2017-01-26");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-yang-push@2019-09-09");
    skipNetconfChunk(processOutput);
    SEND_MODULE(curMsgId++, "ietf-yang-patch@2017-02-22");
    skipNetconfChunk(processOutput);
    SEND_OK(curMsgId++);
    skipNetconfChunk(processOutput);
    SEND_OK(curMsgId++);
    x.join();
}
