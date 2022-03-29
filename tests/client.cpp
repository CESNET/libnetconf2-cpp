/*
 * Copyright (C) 2021 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/process.hpp>
#include <iostream>
#include <doctest/doctest.h>
#include <filesystem>
#include <functional>
#include <libnetconf2-cpp/netconf-client.hpp>
#include <optional>
#include <thread>
#include "UniqueResource.hpp"
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
    std::function<std::optional<libyang::DataNode>(std::unique_ptr<libnetconf::client::Session>& session)> testedFunctionality;
    std::string replyData;
    std::string expectedJSON;

    DOCTEST_SUBCASE("get-data")
    {
        testedFunctionality = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            return session->getData(libnetconf::NmdaDatastore::Running);
        };

        DOCTEST_SUBCASE("some data")
        {
            replyData = createNmdaDataReply(R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)"s);
            expectedJSON = R"({
  "example-schema:myLeaf": "AHOJ"
}
)";
        }

        DOCTEST_SUBCASE("no data")
        {
            replyData = createNmdaDataReply("");
            expectedJSON = "";
        }
    }

    DOCTEST_SUBCASE("rpc")
    {
        testedFunctionality = [] (std::unique_ptr<libnetconf::client::Session>& session) {
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
            expectedJSON = R"({
  "example-schema:myRpc": {
    "myOutput": "LOL"
  }
}
)";
        }
    }

    DOCTEST_SUBCASE("get")
    {
        testedFunctionality = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            return session->get();
        };

        DOCTEST_SUBCASE("some data")
        {
            replyData = createGetReply(R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)"s);
            expectedJSON = R"({
  "example-schema:myLeaf": "AHOJ"
}
)";
        }

        DOCTEST_SUBCASE("no data")
        {
            replyData = createGetReply("");
            expectedJSON = "";
        }
    }

    DOCTEST_SUBCASE("copyConfig")
    {
        testedFunctionality = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            session->copyConfig(libnetconf::Datastore::Running, libnetconf::Datastore::Startup);
            return std::nullopt;
        };

        replyData = mock_server::OK_REPLY;
    }

    DOCTEST_SUBCASE("copyConfigFromString")
    {
        testedFunctionality = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            session->copyConfigFromString(libnetconf::Datastore::Running, R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)");
            return std::nullopt;
        };

        replyData = mock_server::OK_REPLY;
    }

    DOCTEST_SUBCASE("editData")
    {
        testedFunctionality = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            session->editData(libnetconf::NmdaDatastore::Running, R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)");
            return std::nullopt;
        };

        replyData = mock_server::OK_REPLY;
    }

    DOCTEST_SUBCASE("editConfig")
    {
        testedFunctionality = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            session->editConfig(
                libnetconf::Datastore::Running,
                libnetconf::EditDefaultOp::Merge,
                libnetconf::EditTestOpt::TestSet,
                libnetconf::EditErrorOpt::Rollback,
                R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)");
            return std::nullopt;
        };

        replyData = mock_server::OK_REPLY;
    }

    libnetconf::client::setLogLevel(libnetconf::LogLevel::Debug);
    auto x = std::jthread{[&testedFunctionality, &expectedJSON, &processInput, &processOutput] {
        auto session = libnetconf::client::Session::connectFd(processInput.pipe().native_source(), processOutput.pipe().native_sink());
        auto dataNode = testedFunctionality(session);
        std::string actualJSON;
        if (dataNode) {
            actualJSON = dataNode->printStr(libyang::DataFormat::JSON, libyang::PrintFlags::WithSiblings).value();
        }
        REQUIRE(actualJSON == expectedJSON);

    }};

    auto testFailureHandler = make_unique_resource([] {}, [&] {
        // Check whether we're calling this while throwing: this indicates that there has been some expectation failure
        // from doctest. Such expectation failure may cause the client thread to block, because we didn't give it a
        // reply. In this case we will close the pipes. This does make the client thread throw an unhandled exception,
        // but that's fine, because the test failed anyway.
        if (std::uncaught_exceptions()) {
            processInput.pipe().close();
            processOutput.pipe().close();
        }
    });

    mock_server::handleSessionStart(curMsgId, processInput, processOutput);

    mock_server::skipNetconfChunk(processOutput);
    mock_server::sendRpcReply(curMsgId, processInput, replyData);

    // For <close-session>
    mock_server::skipNetconfChunk(processOutput, {"<close-session"});
    mock_server::sendRpcReply(curMsgId, processInput, mock_server::OK_REPLY);
}
