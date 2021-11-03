/*
 * Copyright (C) 2021 CESNET, https://photonics.cesnet.cz/
 *
 * Written by Václav Kubernát <kubernat@cesnet.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <doctest/doctest.h>
#include <libnetconf2-cpp/Context.hpp>
#include <optional>
#include "test_vars.hpp"

TEST_CASE("context")
{
    std::optional<libnetconf2::Context> ctx{std::in_place};
    REQUIRE(ctx);
}
