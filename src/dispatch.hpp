/// \author Jian Wen
/// \date 2021/02/02

#pragma once

#include "types.hpp"

#include <queue>

template <typename RouterFunc>
void assign_trips_through_insertion_heuristics(
    std::queue<const std::vector<Trip>::iterator> &pending_trips,
    RouterFunc &_router_func);

// Implementation is put in a separate file for clarity and maintainability.
#include "dispatch_impl.hpp"
