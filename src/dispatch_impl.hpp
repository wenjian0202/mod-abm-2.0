/// \author Jian Wen
/// \date 2021/02/02

#pragma once

#include "dispatch.hpp"

#include <fmt/format.h>

template <typename RouterFunc>
void assign_trips_through_insertion_heuristics(
    std::queue<const std::vector<Trip>::iterator> &pending_trips,
    RouterFunc &_router_func)
{
    fmt::print("[DEBUG] Assigning trips to vehicles through insertion heuristics.\n");

    return;
}