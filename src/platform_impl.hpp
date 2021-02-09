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

namespace
{

    void truncate_step_by_time(Step &step, double time_s)
    {
        assert(step.poses.size() >= 2 && "Input step in truncate_step_by_time() must have at least 2 poses!");
        assert(step.distance_m > 0 && "Input step's distance in truncate_step_by_time() must be positive!");
        assert(step.duration_s > 0 && "Input step's duration in truncate_step_by_time() must be positive!");

        auto ratio = time_s / step.duration_s;

        assert(ratio >= 0 && ratio < 1 && "Ratio in truncate_step_by_time() must be within [0, 1)!");

        // Early return.
        if (ratio == 0.0)
        {
            return;
        }

        // Get the total distance of the step. We use Mahhantan distance for simplicity.
        auto total_dist = 0.0;
        for (auto i = 0; i < step.poses.size() - 1; i++)
        {
            total_dist += abs(step.poses[i].lat - step.poses[i + 1].lat) + abs(step.poses[i].lon - step.poses[i + 1].lon);
        }

        // Compute the distance to be truncated.
        const auto truncated_dist = total_dist * ratio;

        // Iterate through the poses for the target distance.
        auto accumulated_dist = 0.0;
        for (auto i = 0; i < step.poses.size() - 1; i++)
        {
            auto dist = abs(step.poses[i].lat - step.poses[i + 1].lat) + abs(step.poses[i].lon - step.poses[i + 1].lon);

            if (accumulated_dist + dist > truncated_dist)
            {
                auto subratio = (truncated_dist - accumulated_dist) / dist;

                assert(subratio >= 0 && subratio < 1 && "Ratio in truncate_step_by_time() must be within [0, 1)!");

                step.poses[i].lon = step.poses[i].lon + subratio * (step.poses[i + 1].lon - step.poses[i].lon);
                step.poses[i].lat = step.poses[i].lat + subratio * (step.poses[i + 1].lat - step.poses[i].lat);

                step.poses.erase(step.poses.begin(), step.poses.begin() + i);

                break;
            }

            accumulated_dist += dist;
        }

        step.distance_m *= ratio;
        step.duration_s *= ratio;

        assert(step.poses.size() >= 2 && "Output step in truncate_step_by_time() must have at least 2 poses!");
        assert(step.distance_m > 0 && "Output step's distance in truncate_step_by_time() must be positive!");
        assert(step.duration_s > 0 && "Output step's duration in truncate_step_by_time() must be positive!");
    }

    void truncate_leg_by_time(Leg &leg, double time_s)
    {
        assert(leg.steps.size() >= 1 && "Input leg in truncate_leg_by_time() must have at least 1 step!");
        assert(leg.distance_m > 0 && "Input leg's distance in truncate_leg_by_time() must be positive!");
        assert(leg.duration_s > 0 && "Input leg's duration in truncate_leg_by_time() must be positive!");

        assert(time_s >= 0 && "Time in truncate_leg_by_time() must be non negative!");
        assert(time_s < leg.duration_s && "Time in truncate_leg_by_time() must be less than leg's duration!");

        while (!leg.steps.empty())
        {
            auto &step = leg.steps.front();

            // If we can finish this step within the time, remove the entire step.
            if (step.duration_s <= time_s)
            {
                time_s -= step.duration_s;
                leg.steps.erase(leg.steps.begin());

                continue;
            }

            truncate_step_by_time(step, time_s);
            break;
        }

        // Recalculate the total duration and distance.
        leg.distance_m = 0.0;
        leg.duration_s = 0.0;
        for (const auto &step : leg.steps)
        {
            leg.distance_m += step.distance_m;
            leg.duration_s += step.duration_s;
        }

        assert(leg.steps.size() >= 1 && "Output leg in truncate_leg_by_time() must have at least 1 step!");
        assert(leg.distance_m > 0 && "Output leg's distance in truncate_leg_by_time() must be positive!");
        assert(leg.duration_s > 0 && "Output leg's duration in truncate_leg_by_time() must be positive!");
    }

    void truncate_route_by_time(Route &route, double time_s)
    {
        assert(route.legs.size() >= 1 && "Input route in truncate_route_by_time() must have at least 1 leg!");
        assert(route.distance_m > 0 && "Input route's distance in truncate_route_by_time() must be positive!");
        assert(route.duration_s > 0 && "Input route's duration in truncate_route_by_time() must be positive!");

        assert(time_s >= 0 && "Time in truncate_route_by_time() must be non negative!");
        assert(time_s < route.duration_s && "Time in truncate_route_by_time() must be less than route's duration!");

        while (!route.legs.empty())
        {
            auto &leg = route.legs.front();

            // If we can finish this leg within the time, remove the entire leg.
            if (leg.duration_s <= time_s)
            {
                time_s -= leg.duration_s;

                route.distance_m -= leg.distance_m;
                route.duration_s -= leg.duration_s;

                route.legs.erase(route.legs.begin());
                continue;
            }

            truncate_leg_by_time(leg, time_s);
            break;
        }

        // Recalculate the total duration and distance.
        route.distance_m = 0.0;
        route.duration_s = 0.0;
        for (const auto &leg : route.legs)
        {
            route.distance_m += leg.distance_m;
            route.duration_s += leg.duration_s;
        }

        assert(route.legs.size() >= 1 && "Output route in truncate_route_by_time() must have at least 1 step!");
        assert(route.distance_m > 0 && "Output route's distance in truncate_route_by_time() must be positive!");
        assert(route.duration_s > 0 && "Output route's duration in truncate_route_by_time() must be positive!");
    }

    void advance_vehicle(Vehicle &vehicle, std::vector<Trip> &trips, double system_time_s, double time_s)
    {
        while (!vehicle.waypoints.empty())
        {
            auto &wp = vehicle.waypoints.front();

            // If we can finish this waypoint within the time.
            if (wp.route.duration_s <= time_s)
            {
                system_time_s += wp.route.duration_s;
                time_s -= wp.route.duration_s;

                vehicle.pos = wp.pos;

                if (wp.op == WaypointOp::PICKUP)
                {
                    trips[wp.trip_id].pickup_time_s = system_time_s;
                    vehicle.load++;
                }
                else if (wp.op == WaypointOp::DROPOFF)
                {
                    trips[wp.trip_id].dropoff_time_s = system_time_s;
                    vehicle.load--;
                }

                vehicle.waypoints.erase(vehicle.waypoints.begin());
                continue;
            }

            // If we can not finish this waypoint, truncate the route.
            truncate_route_by_time(wp.route, time_s);
            vehicle.pos = wp.route.legs.front().steps.front().poses.front();

            return;
        }

        return;
    }

} // namespace

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