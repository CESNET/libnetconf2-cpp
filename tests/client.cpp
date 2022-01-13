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

TEST_CASE("client")
{
    using namespace std::string_literals;
    boost::process::ipstream processOutput;
    boost::process::opstream processInput;

    int curMsgId = 1;

    std::function<std::optional<libyang::DataNode>(std::unique_ptr<libnetconf::client::Session>& session)> testImpl;
    std::string data;
    std::string expected;

    DOCTEST_SUBCASE("get-data")
    {
        testImpl = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            return session->getData(libnetconf::NmdaDatastore::Running);
        };

        DOCTEST_SUBCASE("some data")
        {
            data = createNmdaDataReply(R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)"s);
            expected = R"({
  "example-schema:myLeaf": "AHOJ"
}
)";
        }

        DOCTEST_SUBCASE("no data")
        {
            data = createNmdaDataReply("");
            expected = "";
        }
    }

    DOCTEST_SUBCASE("rpc")
    {
        testImpl = [] (std::unique_ptr<libnetconf::client::Session>& session) {
            auto rpc = session->libyangContext().newPath("/example-schema:myRpcNoOutput");
            return session->rpc_or_action(*rpc.printStr(libyang::DataFormat::XML, libyang::PrintFlags::WithDefaultsImplicitTag));
        };

        DOCTEST_SUBCASE("No output")
        {
            data = "<ok/>";
        }

        DOCTEST_SUBCASE("With output")
        {
            data = R"(<myOutput xmlns="http://example.com">LOL</myOutput>)";
            expected = R"({
  "example-schema:myRpcNoOutput": {
    "myOutput": "LOL"
  }
}
)";
        }
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
    mock_server::sendRpcReply(curMsgId, processInput, data);

    // For <close-session>
    mock_server::skipNetconfChunk(processOutput);
    mock_server::sendOkReply(curMsgId, processInput);
}
