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

    libnetconf::client::setLogLevel(NC_VERB_DEBUG);
    auto x = std::jthread{[&processInput, &processOutput] {
        auto session = libnetconf::client::Session::connectFd(processInput.pipe().native_source(), processOutput.pipe().native_sink());
        auto data = session->getData(libnetconf::NmdaDatastore::Running);
        REQUIRE(data);
        REQUIRE(data->path() == "/example-schema:myLeaf");
    }};

    mock_server::handleSessionStart(curMsgId, processInput, processOutput);

    // For <get-data>
    mock_server::skipNetconfChunk(processOutput);
    const auto exampleData = createNmdaDataReply(R"(<myLeaf xmlns="http://example.com">AHOJ</myLeaf>)"s);
    mock_server::sendRpcReply(curMsgId, processInput, exampleData);

    // For <close-session>
    mock_server::skipNetconfChunk(processOutput);
    mock_server::sendOkReply(curMsgId, processInput);
}
