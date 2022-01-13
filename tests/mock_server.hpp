/*
 * Copyright (C) 2021 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/process.hpp>

namespace mock_server {
void handleSessionStart(int& curMsgId, boost::process::opstream& processInput, boost::process::ipstream& processOutput);
}
