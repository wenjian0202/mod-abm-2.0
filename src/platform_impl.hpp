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
    // Initialize the fleet.
    const auto &fleet_config = platform_config_.mod_system_config.fleet_config;
    Vehicle vehicle{{fleet_config.initial_lon, fleet_config.initial_lat}, fleet_config.veh_capacity, 0, {}};
    vehicles_ = std::vector<Vehicle>(fleet_config.fleet_size, vehicle);

    // Open the output datalog file.
    fout_datalog.open("./datalog/datalog.yml"); 
}

template <typename RouterFunc, typename DemandGeneratorFunc>
Platform<RouterFunc, DemandGeneratorFunc>::~Platform()
{
    // Initialize the fleet.
    fout_datalog.close();
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
    write_to_datalog();

    // System time moves forward by a cycle.
    system_time_s_ += platform_config_.simulation_config.cycle_s;

    fmt::print("[INFO] T = {}: Cycle {} is running.\n",
               system_time_s_,
               static_cast<int>(system_time_s_ / platform_config_.simulation_config.cycle_s));

    // Get trip requests generated during the past cycle.
    auto requests = demand_generator_func_(system_time_s_);

    // Add the requests into the trip list as well as the pending trip queue.
    for (auto &request : requests)
    {
        Trip trip;

        trip.id = trips_.size();
        trip.origin = request.origin;
        trip.destination = request.destination;
        trip.status = TripStatus::REQUESTED;
        trip.request_time_s = request.request_time_s;
        trip.max_dispatch_time_s = request.request_time_s + platform_config_.mod_system_config.request_config.max_dispatch_wait_time_s;
        trip.max_pickup_time_s = request.request_time_s + platform_config_.mod_system_config.request_config.max_pickup_wait_time_s;

        pending_trip_ids_.push(trips_.size());
        trips_.emplace_back(std::move(trip));
    }

    // Dispatch
    dispatch();
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::write_to_datalog()
{
    YAML::Node node;
    node["system_time_s"] = system_time_s_;

    for (const auto& vehicle : vehicles_)
    {
        YAML::Node veh_node;

        YAML::Node pos_node;
        pos_node["lon"] = vehicle.pos.lon;
        pos_node["lat"] = vehicle.pos.lat;
        veh_node["pos"] = std::move(pos_node);

        YAML::Node waypoints_node;
        for (const auto& waypoint : vehicle.waypoints)
        {
            YAML::Node waypoint_node;
            for (const auto& leg : waypoint.route.legs)
            {
                for (const auto& step : leg.steps)
                {
                    for (const auto& pos : step.poses)
                    {
                        YAML::Node leg_node;
                        leg_node["lon"] = pos.lon;
                        leg_node["lat"] = pos.lat;
                        waypoint_node.push_back(std::move(leg_node));
                    }
                }
            }
            waypoints_node.push_back(std::move(waypoint_node));
        }
        veh_node["waypoints"] = std::move(waypoints_node);
        node["vehicles"].push_back(std::move(veh_node));
    }

    YAML::Node node_wrapper;
    node_wrapper.push_back(node);

    fout_datalog << node_wrapper << std::endl;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::dispatch()
{
    fmt::print("[DEBUG] T = {}: Dispatching {} pending trip(s) to vehicles.\n",
               system_time_s_,
               pending_trip_ids_.size());

    // Assign pending trips to vehicles.
    assign_trips_through_insertion_heuristics(pending_trip_ids_, trips_, vehicles_, system_time_s_, router_func_);

    // Reoptimize the assignments for better level of service.
    // (TODO)

    // Rebalance empty vehicles.
    // (TODO)

    return;
}