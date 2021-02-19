/// \author Jian Wen
/// \date 2021/02/02

#pragma once

#include "dispatch.hpp"
#include "platform.hpp"

#include <fmt/format.h>

template <typename RouterFunc, typename DemandGeneratorFunc>
Platform<RouterFunc, DemandGeneratorFunc>::Platform(PlatformConfig _platform_config,
                                                    RouterFunc _router_func,
                                                    DemandGeneratorFunc _demand_generator_func)
    : platform_config_(std::move(_platform_config)), router_func_(std::move(_router_func)),
      demand_generator_func_(std::move(_demand_generator_func)) {
    // Initialize the fleet.
    const auto &fleet_config = platform_config_.mod_system_config.fleet_config;
    Vehicle vehicle{0,
                    {fleet_config.initial_lon, fleet_config.initial_lat},
                    fleet_config.veh_capacity,
                    0,
                    {},
                    0.0,
                    0.0};
    for (auto i = 0; i < fleet_config.fleet_size; i++) {
        vehicle.id = i;
        vehicles_.emplace_back(vehicle);
    }

    // Open the output datalog file.
    const auto &datalog_config = platform_config_.output_config.datalog_config;
    if (datalog_config.output_datalog) {
        fout_datalog.open(datalog_config.path_to_output_datalog);

        fmt::print("[INFO] Open the output datalog file at {}.\n",
                   datalog_config.path_to_output_datalog);
    }
}

template <typename RouterFunc, typename DemandGeneratorFunc>
Platform<RouterFunc, DemandGeneratorFunc>::~Platform() {
    // Close the datalog stream.
    if (fout_datalog.is_open()) {
        fout_datalog.close();
    }
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::run_simulation() {
    // Total simulation time as the sum of warm-up, main simulation, and wind-down.
    auto total_simulation_time_s = platform_config_.simulation_config.warmup_duration_s +
                                   platform_config_.simulation_config.simulation_duration_s +
                                   platform_config_.simulation_config.winddown_duration_s;

    fmt::print("[INFO] Simulation started. Running for total {} seconds.\n",
               total_simulation_time_s);
    auto start = std::chrono::system_clock::now();

    // Run simulation cycle by cycle.
    while (system_time_s_ < total_simulation_time_s) {
        run_cycle();
    }

    if (platform_config_.output_config.datalog_config.output_datalog) {
        write_trips_to_datalog();
    }

    // Create report.
    runtime_ = std::chrono::system_clock::now() - start;
    fmt::print("[INFO] Simulation completed. Creating report.\n");
    create_report();

    return;
};

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::run_cycle() {
    fmt::print("[INFO] T = {}: Cycle {} is running.\n",
               system_time_s_,
               static_cast<int>(system_time_s_ / platform_config_.simulation_config.cycle_s));

    // Check if we are in main simulation and need to update the vehicle statistics.
    bool in_main_simulation =
        system_time_s_ >= platform_config_.simulation_config.warmup_duration_s &&
        system_time_s_ < platform_config_.simulation_config.warmup_duration_s +
                             platform_config_.simulation_config.simulation_duration_s;

    // If we render video, we compute the states of all vehicles for each of the frames in this
    // cycle and write to datalog.
    if (platform_config_.output_config.video_config.render_video && in_main_simulation) {
        const auto frame_time_s = platform_config_.simulation_config.cycle_s /
                                  platform_config_.output_config.video_config.frames_per_cycle;

        for (auto i = 0; i < platform_config_.output_config.video_config.frames_per_cycle; i++) {
            advance_vehicles(frame_time_s);
            write_state_to_datalog();
        }

        fmt::print("[DEBUG] T = {}: Advanced vehicles by {} second(s), creating {} frames.\n",
                   system_time_s_,
                   platform_config_.simulation_config.cycle_s,
                   platform_config_.output_config.video_config.frames_per_cycle);
    }
    // Otherwise, we advance the vehicles by the whole cycle time.
    else {
        advance_vehicles(platform_config_.simulation_config.cycle_s);

        if (platform_config_.output_config.datalog_config.output_datalog && in_main_simulation) {
            write_state_to_datalog();
        }

        fmt::print("[DEBUG] T = {}: Advanced vehicles by {} second(s).\n",
                   system_time_s_,
                   platform_config_.simulation_config.cycle_s);
    }

    // Get trip requests generated during the past cycle.
    auto requests = demand_generator_func_(system_time_s_);

    fmt::print("[DEBUG] T = {}: Generated {} request(s) in this cycle:\n",
               system_time_s_,
               requests.size());

    // Add the requests into the trip list as well as the pending trips.
    std::vector<size_t> pending_trip_ids;
    for (auto &request : requests) {
        assert(request.origin.lon >= platform_config_.area_config.lon_min &&
               request.origin.lon <= platform_config_.area_config.lon_max &&
               "request.origin.lon is out of bound!");
        assert(request.origin.lat >= platform_config_.area_config.lat_min &&
               request.origin.lat <= platform_config_.area_config.lat_max &&
               "request.origin.lat is out of bound!");
        assert(request.destination.lon >= platform_config_.area_config.lon_min &&
               request.destination.lon <= platform_config_.area_config.lon_max &&
               "request.destination.lon is out of bound!");
        assert(request.destination.lat >= platform_config_.area_config.lat_min &&
               request.destination.lat <= platform_config_.area_config.lat_max &&
               "request.destination.lat is out of bound!");

        Trip trip;

        trip.id = trips_.size();
        trip.origin = request.origin;
        trip.destination = request.destination;
        trip.status = TripStatus::REQUESTED;
        trip.request_time_s = request.request_time_s;
        trip.max_pickup_time_s =
            request.request_time_s +
            platform_config_.mod_system_config.request_config.max_pickup_wait_time_s;

        pending_trip_ids.emplace_back(trips_.size());
        trips_.emplace_back(std::move(trip));

        fmt::print(
            "[DEBUG] Trip #{} requested at T = {}, from origin ({}, {}) to destination ({}, {}):\n",
            trip.id,
            trip.request_time_s,
            trip.origin.lon,
            trip.origin.lat,
            trip.destination.lon,
            trip.destination.lat);
    }

    // Dispatch the pending trips.
    dispatch(pending_trip_ids);

    return;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::advance_vehicles(double time_s) {
    // Check if we are in main simulation and need to update the vehicle statistics.
    bool in_main_simulation =
        system_time_s_ >= platform_config_.simulation_config.warmup_duration_s &&
        system_time_s_ < platform_config_.simulation_config.warmup_duration_s +
                             platform_config_.simulation_config.simulation_duration_s;

    // Do it for each of the vehicles independently.
    for (auto &vehicle : vehicles_) {
        advance_vehicle(vehicle, trips_, system_time_s_, time_s, in_main_simulation);
    }

    // Increment the system time.
    system_time_s_ += time_s;

    return;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::dispatch(
    const std::vector<size_t> &pending_trip_ids) {
    fmt::print("[DEBUG] T = {}: Dispatching {} pending trip(s) to vehicles.\n",
               system_time_s_,
               pending_trip_ids.size());

    // Assign pending trips to vehicles.
    assign_trips_through_insertion_heuristics(
        pending_trip_ids, trips_, vehicles_, system_time_s_, router_func_);

    // Reoptimize the assignments for better level of service.
    // (TODO)

    // Rebalance empty vehicles.
    // (TODO)

    return;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::write_state_to_datalog() {
    YAML::Node node;
    node["system_time_s"] = system_time_s_;

    // For each of the vehicles, we write the relavant data in yaml format.
    for (const auto &vehicle : vehicles_) {
        YAML::Node veh_node;

        YAML::Node pos_node;
        pos_node["lon"] = fmt::format("{:.6f}", vehicle.pos.lon);
        pos_node["lat"] = fmt::format("{:.6f}", vehicle.pos.lat);
        veh_node["pos"] = std::move(pos_node);

        YAML::Node waypoints_node;
        for (const auto &waypoint : vehicle.waypoints) {
            YAML::Node waypoint_node;
            auto count = 0;
            for (const auto &leg : waypoint.route.legs) {
                for (const auto &step : leg.steps) {
                    for (const auto &pose : step.poses) {
                        YAML::Node leg_node;
                        leg_node["lon"] = fmt::format("{:.6f}", pose.lon);
                        leg_node["lat"] = fmt::format("{:.6f}", pose.lat);
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
void Platform<RouterFunc, DemandGeneratorFunc>::write_trips_to_datalog() {
    YAML::Node node;
    node["system_time_s"] = system_time_s_;

    // For each of the trips, we write the relavant data in yaml format.
    for (const auto &trip : trips_) {
        if (trip.request_time_s <= platform_config_.simulation_config.warmup_duration_s) {
            continue;
        } else if (trip.request_time_s >
                   platform_config_.simulation_config.warmup_duration_s +
                       platform_config_.simulation_config.simulation_duration_s) {
            break;
        }

        YAML::Node origin_pos_node;
        origin_pos_node["lon"] = fmt::format("{:.6f}", trip.origin.lon);
        origin_pos_node["lat"] = fmt::format("{:.6f}", trip.origin.lat);

        YAML::Node destination_pos_node;
        destination_pos_node["lon"] = fmt::format("{:.6f}", trip.destination.lon);
        destination_pos_node["lat"] = fmt::format("{:.6f}", trip.destination.lat);

        YAML::Node trip_node;
        trip_node["id"] = trip.id;
        trip_node["origin"] = std::move(origin_pos_node);
        trip_node["destination"] = std::move(destination_pos_node);
        trip_node["status"] = to_string(trip.status);
        trip_node["request_time_s"] = trip.request_time_s;
        trip_node["max_pickup_time_s"] = trip.max_pickup_time_s;
        trip_node["pickup_time_s"] = trip.pickup_time_s;
        trip_node["dropoff_time_s"] = trip.dropoff_time_s;

        node["trips"].push_back(std::move(trip_node));
    }

    YAML::Node node_wrapper;
    node_wrapper.push_back(node);

    fout_datalog << node_wrapper << std::endl;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::create_report() {
    fmt::print("-----------------------------------------------------------------------------------"
               "-----------------------------\n");

    // Report the platform configurations
    auto total_simulation_time_s = platform_config_.simulation_config.warmup_duration_s +
                                   platform_config_.simulation_config.simulation_duration_s +
                                   platform_config_.simulation_config.winddown_duration_s;

    fmt::print("# System Configurations\n");
    fmt::print(" - Simulation Config: simulation_duration = {}s ({}s warm-up + {}s main + {}s "
               "wind-down).\n",
               total_simulation_time_s,
               platform_config_.simulation_config.warmup_duration_s,
               platform_config_.simulation_config.simulation_duration_s,
               platform_config_.simulation_config.winddown_duration_s);
    fmt::print(" - Fleet Config: fleet_size = {}, vehicle_capacity = {}.\n",
               platform_config_.mod_system_config.fleet_config.fleet_size,
               platform_config_.mod_system_config.fleet_config.veh_capacity);
    fmt::print(" - Request Config: max_wait_time = {}s.\n",
               platform_config_.mod_system_config.request_config.max_pickup_wait_time_s);
    fmt::print(" - Output Config: output_datalog = {}, render_video = {}.\n",
               platform_config_.output_config.datalog_config.output_datalog,
               platform_config_.output_config.video_config.render_video);

    // Simulation Runtime
    fmt::print("# Simulation Runtime\n");
    fmt::print(" - Runtime: total_runtime = {}s, average_runtime_per_simulated_second = {}.\n",
               runtime_.count(),
               runtime_.count() / total_simulation_time_s);

    // Report trip status
    auto trip_count = 0;
    auto dispatched_trip_count = 0;
    auto completed_trip_count = 0;
    auto total_wait_time_s = 0.0;
    auto total_travel_time_s = 0.0;

    for (const auto &trip : trips_) {
        if (trip.request_time_s <= platform_config_.simulation_config.warmup_duration_s) {
            continue;
        } else if (trip.request_time_s >
                   platform_config_.simulation_config.warmup_duration_s +
                       platform_config_.simulation_config.simulation_duration_s) {
            break;
        }

        trip_count++;

        if (trip.status == TripStatus::WALKAWAY) {
            continue;
        }

        dispatched_trip_count++;

        if (trip.status == TripStatus::DROPPED_OFF) {
            completed_trip_count++;
            total_wait_time_s += trip.pickup_time_s - trip.request_time_s;
            total_travel_time_s += trip.dropoff_time_s - trip.pickup_time_s;
        }
    }

    fmt::print("# Trips\n");
    fmt::print(
        " - Total Trips: requested = {} (of which {} dispatched [{}%] + {} walked away [{}%]).\n",
        trip_count,
        dispatched_trip_count,
        100.0 * dispatched_trip_count / trip_count,
        trip_count - dispatched_trip_count,
        100.0 - 100.0 * dispatched_trip_count / trip_count);
    fmt::print(" - Travel Time: completed = {}.", completed_trip_count);
    if (completed_trip_count > 0) {
        fmt::print(" average_wait_time = {}s, average_travel_time = {}s.\n",
                   total_wait_time_s / completed_trip_count,
                   total_travel_time_s / completed_trip_count);
    } else {
        fmt::print(" PLEASE USE LONGER SIMULATION DURATION TO BE ABLE TO COMPLETE TRIPS!\n");
    }

    // Report vehicle status
    auto total_dist_traveled_m = 0.0;
    auto total_loaded_dist_traveled_m = 0.0;

    for (const auto &vehicle : vehicles_) {
        total_dist_traveled_m += vehicle.dist_traveled_m;
        total_loaded_dist_traveled_m += vehicle.loaded_dist_traveled_m;
    }

    fmt::print("# Vehicles\n");
    fmt::print(
        " - Distance: average_distance_traveled = {}m. average_distance_traveled_per_hour = {}m.\n",
        total_dist_traveled_m / vehicles_.size(),
        total_dist_traveled_m / vehicles_.size() /
            platform_config_.simulation_config.simulation_duration_s * 3600);
    fmt::print(" - Load: average_load = {}.\n",
               total_loaded_dist_traveled_m / total_dist_traveled_m);

    fmt::print("-----------------------------------------------------------------------------------"
               "-----------------------------\n");
    return;
}
