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
                    0,
                    0};
    for (auto i = 0; i < fleet_config.fleet_size; i++) {
        vehicle.id = i;
        vehicles_.emplace_back(vehicle);
    }

    // Initialize the simulation times.
    system_time_ms_ = 0;
    cycle_ms_ = static_cast<uint64_t>(platform_config_.simulation_config.cycle_s * 1000);
    if (platform_config_.output_config.video_config.render_video) {
        assert(cycle_ms_ % platform_config_.output_config.video_config.frames_per_cycle == 0 &&
               "The cycle time (in milliseconds) must be divisible by frames_per_cycle!");
        frame_ms_ = cycle_ms_ / platform_config_.output_config.video_config.frames_per_cycle;
    } else {
        frame_ms_ = cycle_ms_;
    }
    main_sim_start_time_ms_ =
        static_cast<uint64_t>(platform_config_.simulation_config.warmup_duration_s * 1000);
    main_sim_end_time_ms_ =
        main_sim_start_time_ms_ +
        static_cast<uint64_t>(platform_config_.simulation_config.simulation_duration_s * 1000);
    system_shutdown_time_ms_ =
        main_sim_end_time_ms_ +
        static_cast<uint64_t>(platform_config_.simulation_config.winddown_duration_s * 1000);

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
    fmt::print("[INFO] Simulation started. Running for total {} seconds.\n",
               system_shutdown_time_ms_ / 1000.0);
    auto start = std::chrono::system_clock::now();

    // Run simulation cycle by cycle.
    while (system_time_ms_ < system_shutdown_time_ms_) {
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
    fmt::print("[INFO] T = {}s: Cycle {} is running.\n",
               system_time_ms_ / 1000.0,
               system_time_ms_ / cycle_ms_);

    // Advance the vehicles frame by frame.
    for (auto i = 0; i < cycle_ms_; i += frame_ms_) {
        advance_vehicles();

        if (platform_config_.output_config.datalog_config.output_datalog &&
            system_time_ms_ >= main_sim_start_time_ms_ && system_time_ms_ < main_sim_end_time_ms_) {
            write_state_to_datalog();
        }

        fmt::print("[DEBUG] T = {}s: Advanced vehicles by {}s.\n",
                   system_time_ms_ / 1000.0,
                   cycle_ms_ / 1000.0);
    }

    // Generate trips.
    const auto pending_trip_ids = generate_trips();

    // Dispatch the pending trips.
    dispatch(pending_trip_ids);

    return;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::advance_vehicles() {
    // Do it for each of the vehicles independently.
    for (auto &vehicle : vehicles_) {
        advance_vehicle(vehicle,
                        trips_,
                        system_time_ms_,
                        frame_ms_,
                        system_time_ms_ >= main_sim_start_time_ms_ &&
                            system_time_ms_ < main_sim_end_time_ms_);
    }

    // Increment the system time.
    system_time_ms_ += frame_ms_;

    return;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
std::vector<size_t> Platform<RouterFunc, DemandGeneratorFunc>::generate_trips() {
    // Get trip requests generated during the past cycle.
    auto requests = demand_generator_func_(system_time_ms_);

    fmt::print("[DEBUG] T = {}s: Generated {} request(s) in this cycle:\n",
               system_time_ms_ / 1000.0,
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
        trip.request_time_ms = request.request_time_ms;
        trip.max_pickup_time_ms =
            request.request_time_ms +
            static_cast<uint64_t>(
                platform_config_.mod_system_config.request_config.max_pickup_wait_time_s * 1000);

        pending_trip_ids.emplace_back(trips_.size());
        trips_.emplace_back(std::move(trip));

        fmt::print("[DEBUG] Trip #{} requested at T = {}s, from origin ({}, {}) to destination "
                   "({}, {}):\n",
                   trip.id,
                   trip.request_time_ms / 1000.0,
                   trip.origin.lon,
                   trip.origin.lat,
                   trip.destination.lon,
                   trip.destination.lat);
    }

    return pending_trip_ids;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::dispatch(
    const std::vector<size_t> &pending_trip_ids) {
    fmt::print("[DEBUG] T = {}s: Dispatching {} pending trip(s) to vehicles.\n",
               system_time_ms_ / 1000.0,
               pending_trip_ids.size());

    // Assign pending trips to vehicles.
    assign_trips_through_insertion_heuristics(
        pending_trip_ids, trips_, vehicles_, system_time_ms_, router_func_);

    // Reoptimize the assignments for better level of service.
    // (TODO)

    // Rebalance empty vehicles.
    // (TODO)

    return;
}

template <typename RouterFunc, typename DemandGeneratorFunc>
void Platform<RouterFunc, DemandGeneratorFunc>::write_state_to_datalog() {
    YAML::Node node;
    node["system_time_ms"] = system_time_ms_;

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
    node["system_time_ms"] = system_time_ms_;

    // For each of the trips, we write the relavant data in yaml format.
    for (const auto &trip : trips_) {
        if (trip.request_time_ms <= main_sim_start_time_ms_) {
            continue;
        } else if (trip.request_time_ms > main_sim_end_time_ms_) {
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
        trip_node["request_time_ms"] = trip.request_time_ms;
        trip_node["max_pickup_time_ms"] = trip.max_pickup_time_ms;
        trip_node["pickup_time_ms"] = trip.pickup_time_ms;
        trip_node["dropoff_time_ms"] = trip.dropoff_time_ms;

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
    fmt::print("# System Configurations\n");
    fmt::print(
        " - Simulation Config: simulation_duration = {}s (main simulation between {}s and {}s).\n",
        system_shutdown_time_ms_ / 1000.0,
        main_sim_start_time_ms_ / 1000.0,
        main_sim_end_time_ms_ / 1000.0);
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
               runtime_.count() * 1000 / system_shutdown_time_ms_);

    // Report trip status
    auto trip_count = 0;
    auto dispatched_trip_count = 0;
    auto completed_trip_count = 0;
    auto total_wait_time_ms = 0;
    auto total_travel_time_ms = 0;

    for (const auto &trip : trips_) {
        if (trip.request_time_ms <= main_sim_start_time_ms_) {
            continue;
        } else if (trip.request_time_ms > main_sim_end_time_ms_) {
            break;
        }

        trip_count++;

        if (trip.status == TripStatus::WALKAWAY) {
            continue;
        }

        dispatched_trip_count++;

        if (trip.status == TripStatus::DROPPED_OFF) {
            completed_trip_count++;
            total_wait_time_ms += trip.pickup_time_ms - trip.request_time_ms;
            total_travel_time_ms += trip.dropoff_time_ms - trip.pickup_time_ms;
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
                   total_wait_time_ms / 1000.0 / completed_trip_count,
                   total_travel_time_ms / 1000.0 / completed_trip_count);
    } else {
        fmt::print(" PLEASE USE LONGER SIMULATION DURATION TO BE ABLE TO COMPLETE TRIPS!\n");
    }

    // Report vehicle status
    auto total_dist_traveled_mm = 0;
    auto total_loaded_dist_traveled_mm = 0;

    for (const auto &vehicle : vehicles_) {
        total_dist_traveled_mm += vehicle.dist_traveled_mm;
        total_loaded_dist_traveled_mm += vehicle.loaded_dist_traveled_mm;
    }

    fmt::print("# Vehicles\n");
    fmt::print(
        " - Distance: average_distance_traveled = {}m. average_distance_traveled_per_hour = {}m.\n",
        total_dist_traveled_mm / 1000.0 / vehicles_.size(),
        total_dist_traveled_mm / vehicles_.size() * 3600.0 /
            (main_sim_end_time_ms_ - main_sim_start_time_ms_));
    fmt::print(" - Load: average_load = {}.\n",
               total_loaded_dist_traveled_mm * 1.0 / total_dist_traveled_mm);

    fmt::print("-----------------------------------------------------------------------------------"
               "-----------------------------\n");
    return;
}
