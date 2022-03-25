/*
 * Copyright (C) 2022 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

namespace libnetconf {
enum class LogLevel {
    Error,
    Warning,
    Verbose,
    Debug,
    DebugLowLvl
};

enum class NmdaDatastore {
    Startup,
    Running,
    Candidate,
    Operational
};

enum class Datastore {
    Error = 0,
    Config,
    Url,
    Running,
    Startup,
    Candidate
};

enum class EditDefaultOp {
    Unknown = 0,
    Merge,
    Replace,
    None
};

enum class EditTestOpt {
    Unknown = 0,
    TestSet,
    Set,
    Test
};

enum class EditErrorOpt {
    Unknown = 0,
    Stop,
    Continue,
    Rollback
};
}
