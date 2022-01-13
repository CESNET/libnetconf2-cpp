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

TEST_CASE("client")
{
    boost::process::ipstream processOutput;
    boost::process::opstream processInput;

    int curMsgId = 1;

    libnetconf::client::setLogLevel(NC_VERB_DEBUG);
    auto x = std::jthread{[&processInput, &processOutput] {
        auto session = libnetconf::client::Session::connectFd(processInput.pipe().native_source(), processOutput.pipe().native_sink());
    }};

    mock_server::handleSessionStart(curMsgId, processInput, processOutput);
}
