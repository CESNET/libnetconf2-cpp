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
#include <functional>
#include <libnetconf2-cpp/netconf-client.hpp>
#include <optional>
#include <thread>
#include "mock_server.hpp"
#include "test_vars.hpp"

auto createNmdaDataReply(const std::string& input)
{
    return R"(<data xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-nmda">)" + input + "</data>";
}

auto createGetReply(const std::string& input)
{
    return R"(<data xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">)" + input + "</data>";
}

TEST_CASE("client")
{
    using namespace std::string_literals;
    boost::process::ipstream processOutput;
    boost::process::opstream processInput;

    int curMsgId = 1;

    // This std::function is supposed to call the tested functionality (the tested method in the libnetconf2 client
    // session). The function should return:
    // 1) libyang::DataNode, if the operation supports that (e.g. get-data, rpc). std::nullopt if operation supports
    //    returning data, but returned none.
    // 2) std::nullopt, if the operation doesn't return any data (e.g. copy-config)
    std::function<std::optional<libyang::DataNode>(std::unique_ptr<libnetconf::client::Session>& session)> testImpl;
    std::string replyData;
    std::string expected;

    DOCTEST_SUBCASE("get-data")
    {
        testImpl = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            return session->getData(libnetconf::NmdaDatastore::Running);
        };

        DOCTEST_SUBCASE("some data")
        {
            replyData = createNmdaDataReply(R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)"s);
            expected = R"({
  "example-schema:myLeaf": "AHOJ"
}
)";
        }

        DOCTEST_SUBCASE("no data")
        {
            replyData = createNmdaDataReply("");
            expected = "";
        }
    }

    DOCTEST_SUBCASE("rpc")
    {
        testImpl = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            auto rpc = session->libyangContext().newPath("/example-schema:myRpc");
            return session->rpc_or_action(*rpc.printStr(libyang::DataFormat::XML, libyang::PrintFlags::WithDefaultsImplicitTag));
        };

        DOCTEST_SUBCASE("No output")
        {
            replyData = mock_server::OK_REPLY;
        }

        DOCTEST_SUBCASE("With output")
        {
            replyData = R"(<myOutput xmlns="http://example.com">LOL</myOutput>)";
            expected = R"({
  "example-schema:myRpc": {
    "myOutput": "LOL"
  }
}
)";
        }
    }

    DOCTEST_SUBCASE("get")
    {
        testImpl = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            return session->get();
        };

        DOCTEST_SUBCASE("some data")
        {
            replyData = createGetReply(R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)"s);
            expected = R"({
  "example-schema:myLeaf": "AHOJ"
}
)";
        }

        DOCTEST_SUBCASE("no data")
        {
            replyData = createGetReply("");
            expected = "";
        }
    }

    DOCTEST_SUBCASE("copyConfig")
    {
        testImpl = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            session->copyConfig(NC_DATASTORE_RUNNING, NC_DATASTORE_STARTUP);
            return std::nullopt;
        };

        replyData = mock_server::OK_REPLY;
    }

    DOCTEST_SUBCASE("copyConfigFromString")
    {
        testImpl = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            session->copyConfigFromString(NC_DATASTORE_RUNNING, R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)");
            return std::nullopt;
        };

        replyData = mock_server::OK_REPLY;
    }

    DOCTEST_SUBCASE("editData")
    {
        testImpl = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            session->editData(libnetconf::NmdaDatastore::Running, R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)");
            return std::nullopt;
        };

        replyData = mock_server::OK_REPLY;
    }

    DOCTEST_SUBCASE("editConfig")
    {
        testImpl = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            session->editConfig(
                NC_DATASTORE_RUNNING,
                NC_RPC_EDIT_DFLTOP_MERGE,
                NC_RPC_EDIT_TESTOPT_TESTSET,
                NC_RPC_EDIT_ERROPT_ROLLBACK,
                R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)");
            return std::nullopt;
        };

        replyData = mock_server::OK_REPLY;
    }

    libnetconf::client::setLogLevel(NC_VERB_DEBUG);
    auto x = std::jthread{[&testImpl, &expected, &processInput, &processOutput] {
        auto session = libnetconf::client::Session::connectFd(processInput.pipe().native_source(), processOutput.pipe().native_sink());
        auto dataNode = testImpl(session);
        std::string toTest;
        if (dataNode) {
            toTest = dataNode->printStr(libyang::DataFormat::JSON, libyang::PrintFlags::WithSiblings).value();
        }
        REQUIRE(toTest == expected);

    }};

    mock_server::handleSessionStart(curMsgId, processInput, processOutput);

    mock_server::skipNetconfChunk(processOutput);
    mock_server::sendRpcReply(curMsgId, processInput, replyData);

    // For <close-session>
    mock_server::skipNetconfChunk(processOutput);
    mock_server::sendRpcReply(curMsgId, processInput, mock_server::OK_REPLY);
}
