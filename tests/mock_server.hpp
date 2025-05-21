/*
 * Copyright (C) 2021 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/version.hpp>
#if BOOST_VERSION < 108800
#include <boost/process.hpp>
#else
#define BOOST_PROCESS_VERSION 1
#include <boost/process/v1/pipe.hpp>
#endif

#include <string>

namespace mock_server {
void skipNetconfChunk(boost::process::ipstream& processOutput, const std::vector<std::string>& mustContain = {});
void sendRpcReply(int msgId, boost::process::opstream& processInput, std::string data);
void sendOkReply(int msgId, boost::process::opstream& processInput);
void handleSessionStart(int& curMsgId, boost::process::opstream& processInput, boost::process::ipstream& processOutput);

const auto OK_REPLY = "<ok/>";

}
