/*
 * Copyright (C) 2022 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <libnetconf2-cpp/Enum.hpp>
#include <libnetconf2/log.h>
#include <libnetconf2/messages_client.h>

namespace libnetconf::utils {
constexpr NC_VERB_LEVEL toLogLevel(const LogLevel level)
{
    return static_cast<NC_VERB_LEVEL>(level);
}

constexpr LogLevel toLogLevel(const NC_VERB_LEVEL level)
{
    return static_cast<LogLevel>(level);
}
// These tests ensure that I used the right numbers when defining my enum.
static_assert(LogLevel::Error == toLogLevel(NC_VERB_LEVEL::NC_VERB_ERROR));
static_assert(LogLevel::Warning == toLogLevel(NC_VERB_LEVEL::NC_VERB_WARNING));
static_assert(LogLevel::Verbose == toLogLevel(NC_VERB_LEVEL::NC_VERB_VERBOSE));
static_assert(LogLevel::Debug == toLogLevel(NC_VERB_LEVEL::NC_VERB_DEBUG));
static_assert(LogLevel::DebugLowLvl == toLogLevel(NC_VERB_LEVEL::NC_VERB_DEBUG_LOWLVL));

constexpr NC_DATASTORE toDatastore(const Datastore ds)
{
    return static_cast<NC_DATASTORE>(ds);
}
static_assert(toDatastore(Datastore::Error) == NC_DATASTORE::NC_DATASTORE_ERROR);
static_assert(toDatastore(Datastore::Config) == NC_DATASTORE::NC_DATASTORE_CONFIG);
static_assert(toDatastore(Datastore::Url) == NC_DATASTORE::NC_DATASTORE_URL);
static_assert(toDatastore(Datastore::Running) == NC_DATASTORE::NC_DATASTORE_RUNNING);
static_assert(toDatastore(Datastore::Startup) == NC_DATASTORE::NC_DATASTORE_STARTUP);
static_assert(toDatastore(Datastore::Candidate) == NC_DATASTORE::NC_DATASTORE_CANDIDATE);

constexpr NC_RPC_EDIT_DFLTOP toDefaultOp(const EditDefaultOp ds)
{
    return static_cast<NC_RPC_EDIT_DFLTOP>(ds);
}

static_assert(toDefaultOp(EditDefaultOp::Unknown) == NC_RPC_EDIT_DFLTOP::NC_RPC_EDIT_DFLTOP_UNKNOWN);
static_assert(toDefaultOp(EditDefaultOp::Merge) == NC_RPC_EDIT_DFLTOP::NC_RPC_EDIT_DFLTOP_MERGE);
static_assert(toDefaultOp(EditDefaultOp::Replace) == NC_RPC_EDIT_DFLTOP::NC_RPC_EDIT_DFLTOP_REPLACE);
static_assert(toDefaultOp(EditDefaultOp::None) == NC_RPC_EDIT_DFLTOP::NC_RPC_EDIT_DFLTOP_NONE);

constexpr NC_RPC_EDIT_TESTOPT toTestOpt(const EditTestOpt ds)
{
    return static_cast<NC_RPC_EDIT_TESTOPT>(ds);
}

static_assert(toTestOpt(EditTestOpt::Unknown) == NC_RPC_EDIT_TESTOPT::NC_RPC_EDIT_TESTOPT_UNKNOWN);
static_assert(toTestOpt(EditTestOpt::TestSet) == NC_RPC_EDIT_TESTOPT::NC_RPC_EDIT_TESTOPT_TESTSET);
static_assert(toTestOpt(EditTestOpt::Set) == NC_RPC_EDIT_TESTOPT::NC_RPC_EDIT_TESTOPT_SET);
static_assert(toTestOpt(EditTestOpt::Test) == NC_RPC_EDIT_TESTOPT::NC_RPC_EDIT_TESTOPT_TEST);

constexpr NC_RPC_EDIT_ERROPT toErrorOpt(const EditErrorOpt ds)
{
    return static_cast<NC_RPC_EDIT_ERROPT >(ds);
}

static_assert(toErrorOpt(EditErrorOpt::Unknown) == NC_RPC_EDIT_ERROPT::NC_RPC_EDIT_ERROPT_UNKNOWN);
static_assert(toErrorOpt(EditErrorOpt::Stop) == NC_RPC_EDIT_ERROPT::NC_RPC_EDIT_ERROPT_STOP);
static_assert(toErrorOpt(EditErrorOpt::Continue) == NC_RPC_EDIT_ERROPT::NC_RPC_EDIT_ERROPT_CONTINUE);
static_assert(toErrorOpt(EditErrorOpt::Rollback) == NC_RPC_EDIT_ERROPT::NC_RPC_EDIT_ERROPT_ROLLBACK);
}
