/*
 * Copyright (C) 2021 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/process.hpp>
#include <string>

namespace mock_server {
void skipNetconfChunk(boost::process::ipstream& processOutput);
void sendRpcReply(int msgId, boost::process::opstream& processInput, std::string data);
void sendOkReply(int msgId, boost::process::opstream& processInput);
void handleSessionStart(int& curMsgId, boost::process::opstream& processInput, boost::process::ipstream& processOutput);

const auto OK_REPLY = "<ok/>";

}
