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
    if (platform_config_.output_config.datalog_config.output_datalog)
    {
        fout_datalog.open(platform_config_.output_config.datalog_config.path_to_output_datalog);

        fmt::print("[INFO] Open the output datalog file at {}.\n", platform_config_.output_config.datalog_config.path_to_output_datalog);
    }
}

template <typename RouterFunc, typename DemandGeneratorFunc>
Platform<RouterFunc, DemandGeneratorFunc>::~Platform()
{
    // Close the datalog stream.
    if (platform_config_.output_config.datalog_config.output_datalog)
    {
        fout_datalog.close();
    }
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::run_simulation()
{
    // Total simulation time as the sum of warm-up, main simulation, and wind-down.
    auto total_simulation_time_s = platform_config_.simulation_config.warmup_duration_s + platform_config_.simulation_config.simulation_duration_s + platform_config_.simulation_config.winddown_duration_s;

    fmt::print("[INFO] Simulation started. Running for total {} seconds.\n", total_simulation_time_s);

    while (system_time_s_ < total_simulation_time_s)
    {
        run_cycle();
    }

    return;
};

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::run_cycle()
{
    fmt::print("[INFO] T = {}: Cycle {} is running.\n",
               system_time_s_,
               static_cast<int>(system_time_s_ / platform_config_.simulation_config.cycle_s));

    // If we render video, we compute vehicles' location for each frame and write to datalog.
    if (platform_config_.output_config.video_config.render_video)
    {
        const auto frame_time_s = platform_config_.simulation_config.cycle_s / platform_config_.output_config.video_config.frames_per_cycle;

        for (auto i = 0; i < platform_config_.output_config.video_config.frames_per_cycle; i++)
        {
            advance_vehicles(frame_time_s);
            write_to_datalog();
        }

        fmt::print("[DEBUG] T = {}: Advanced vehicles by {} second(s), creating {} frames.\n",
                   system_time_s_,
                   platform_config_.simulation_config.cycle_s,
                   platform_config_.output_config.video_config.frames_per_cycle);
    }
    // Otherwise, we advance by the whole cycle.
    else
    {
        advance_vehicles(platform_config_.simulation_config.cycle_s);

        if (platform_config_.output_config.datalog_config.output_datalog)
        {
            write_to_datalog();
        }

        fmt::print("[DEBUG] T = {}: Advanced vehicles by {} second(s).\n", system_time_s_, platform_config_.simulation_config.cycle_s);
    }

    // Get trip requests generated during the past cycle.
    auto requests = demand_generator_func_(system_time_s_);

    // Add the requests into the trip list as well as the pending trips.
    std::vector<size_t> pending_trip_ids;
    for (auto &request : requests)
    {
        assert(request.origin.lon >= platform_config_.area_config.lon_min && request.origin.lon <= platform_config_.area_config.lon_max && "request.origin.lon is out of bound!");
        assert(request.origin.lat >= platform_config_.area_config.lat_min && request.origin.lat <= platform_config_.area_config.lat_max && "request.origin.lat is out of bound!");
        assert(request.destination.lon >= platform_config_.area_config.lon_min && request.destination.lon <= platform_config_.area_config.lon_max && "request.destination.lon is out of bound!");
        assert(request.destination.lat >= platform_config_.area_config.lat_min && request.destination.lat <= platform_config_.area_config.lat_max && "request.destination.lat is out of bound!");

        Trip trip;

        trip.id = trips_.size();
        trip.origin = request.origin;
        trip.destination = request.destination;
        trip.status = TripStatus::REQUESTED;
        trip.request_time_s = request.request_time_s;
        trip.max_pickup_time_s = request.request_time_s + platform_config_.mod_system_config.request_config.max_pickup_wait_time_s;

        pending_trip_ids.emplace_back(trips_.size());
        trips_.emplace_back(std::move(trip));
    }

    // Dispatch
    dispatch(pending_trip_ids);
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::advance_vehicles(double time_s)
{
    // Do it for each of the vehicles independently.
    for (auto &vehicle : vehicles_)
    {
        advance_vehicle(vehicle, trips_, system_time_s_, time_s);
    }

    system_time_s_ += time_s;

    return;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::dispatch(const std::vector<size_t> &pending_trip_ids)
{
    fmt::print("[DEBUG] T = {}: Dispatching {} pending trip(s) to vehicles.\n",
               system_time_s_,
               pending_trip_ids.size());

    // Assign pending trips to vehicles.
    assign_trips_through_insertion_heuristics(pending_trip_ids, trips_, vehicles_, system_time_s_, router_func_);

    // Reoptimize the assignments for better level of service.
    // (TODO)

    // Rebalance empty vehicles.
    // (TODO)

    return;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::write_to_datalog()
{
    YAML::Node node;
    node["system_time_s"] = system_time_s_;

    for (const auto &vehicle : vehicles_)
    {
        YAML::Node veh_node;

        YAML::Node pos_node;
        pos_node["lon"] = vehicle.pos.lon;
        pos_node["lat"] = vehicle.pos.lat;
        veh_node["pos"] = std::move(pos_node);

        YAML::Node waypoints_node;
        for (const auto &waypoint : vehicle.waypoints)
        {
            YAML::Node waypoint_node;
            auto count = 0;
            for (const auto &leg : waypoint.route.legs)
            {
                for (const auto &step : leg.steps)
                {
                    for (const auto &pose : step.poses)
                    {
                        YAML::Node leg_node;
                        leg_node["lon"] = pose.lon;
                        leg_node["lat"] = pose.lat;
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