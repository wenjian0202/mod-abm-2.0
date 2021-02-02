/// \author Jian Wen
/// \date 2021/02/02

#pragma once

#include "dispatch.hpp"
#include "platform.hpp"

#include <fmt/format.h>

template <typename RouterFunc, typename DemandGeneratorFunc>
Platform<RouterFunc, DemandGeneratorFunc>::Platform(
    PlatformConfig _platform_config,
    RouterFunc _router_func,
    DemandGeneratorFunc _demand_generator_func)
    : platform_config_(std::move(_platform_config)),
      router_func_(std::move(_router_func)),
      demand_generator_func_(std::move(_demand_generator_func))
{
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::run_simulation()
{
    // Total simulation time as the sum of warm-up, main simulation, and wind-down.
    auto total_simulation_time_s = platform_config_.simulation_config.warmup_duration_s + platform_config_.simulation_config.simulation_duration_s + platform_config_.simulation_config.winddown_duration_s;

    fmt::print("[INFO] Simulation started. Running for {} seconds...\n", total_simulation_time_s);

    while (system_time_s_ < total_simulation_time_s)
    {
        run_cycle();
    }

    return;
};

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::run_cycle()
{
    // System time moves forward by a cycle.
    system_time_s_ += platform_config_.simulation_config.cycle_s;

    fmt::print("[DEBUG] T = {}: Cycle {} is running.\n",
               system_time_s_,
               static_cast<int>(system_time_s_ / platform_config_.simulation_config.cycle_s));

    // Get trip requests generated during the past cycle.
    auto requests = demand_generator_func_(system_time_s_);

    // Add the requests into the trip list as well as the pending trip queue.
    for (auto &request : requests)
    {
        Trip trip;

        trip.origin = request.origin;
        trip.destination = request.destination;
        trip.id = trips_.size();
        trip.status = TripStatus::REQUESTED;
        trip.request_time_s = request.request_time_s;

        trips_.emplace_back(std::move(trip));
        pending_trips_.push(std::prev(trips_.end()));
    }

    // Dispatch
    dispatch();
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::dispatch()
{
    fmt::print("[DEBUG] T = {}: Dispatching {} pending trip(s) to vehicles.\n",
               system_time_s_,
               pending_trips_.size());

    // Assign pending trips to vehicles.
    assign_trips_through_insertion_heuristics(pending_trips_, router_func_);

    // Reoptimize the assignments for better level of service.
    // (TODO)

    // Rebalance empty vehicles.
    // (TODO)

    return;
}