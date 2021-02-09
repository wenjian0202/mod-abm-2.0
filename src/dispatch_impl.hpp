/// \author Jian Wen
/// \date 2021/02/02

#pragma once

#include "dispatch.hpp"

#include <fmt/format.h>

#include <numeric>

template <typename RouterFunc>
void assign_trips_through_insertion_heuristics(
    const std::vector<size_t> &pending_trip_ids,
    std::vector<Trip> &trips,
    std::vector<Vehicle> &vehicles,
    double system_time_s,
    RouterFunc &router_func)
{
    fmt::print("[DEBUG] Assigning trips to vehicles through insertion heuristics.\n");

    // For each trip, we assign it to the best vehicle.
    for (auto trip_id : pending_trip_ids)
    {
        auto &trip = trips[trip_id];

        assign_trip_through_insertion_heuristics(trip, trips, vehicles, system_time_s, router_func);
    }

    return;
}

template <typename RouterFunc>
void assign_trip_through_insertion_heuristics(
    Trip &trip,
    const std::vector<Trip> &trips,
    std::vector<Vehicle> &vehicles,
    double system_time_s,
    RouterFunc &router_func)
{
    InsertionResult res;
    size_t best_vehicle_index = 0;

    // Iterate through all vehicles and find the one with least additional cost.
    const auto num_vehs = vehicles.size();
    for (auto vehicle_index = 0; vehicle_index < num_vehs; vehicle_index++)
    {
        auto &vehicle = vehicles[vehicle_index];
        auto res_this_vehicle = compute_cost_of_inserting_trip_to_vehicle(trip, trips, vehicle, system_time_s, router_func);

        if (res_this_vehicle.success && res_this_vehicle.cost_s < res.cost_s)
        {
            res = std::move(res_this_vehicle);
            best_vehicle_index = vehicle_index;
        }
    }

    // If none of the vehicles can serve the trip, return false.
    if (!res.success)
    {
        fmt::print("[DEBUG] Failed to assign Trip #{}.\n", trip.id);

        return;
    }

    // Insert the trip to the best vehicle.
    auto &best_vehicle = vehicles[best_vehicle_index];
    insert_trip_to_vehicle(trip, best_vehicle, res.pickup_index, res.dropoff_index, router_func);

    fmt::print("[DEBUG] Assigned Trip #{} to Vehicle #{}, which has {} waypoints.\n",
               trip.id, best_vehicle_index, best_vehicle.waypoints.size());

    return;
}

double get_cost_of_waypoints(const std::vector<Waypoint> &waypoints)
{
    auto cost_s = 0.0;
    auto accumulated_time_s = 0.0;

    for (const auto &wp : waypoints)
    {
        accumulated_time_s += wp.route.duration_s;

        if (wp.op == WaypointOp::DROPOFF)
        {
            cost_s += accumulated_time_s;
        }
    }

    return cost_s;
}

bool validate_waypoints(const std::vector<Waypoint> &waypoints, const std::vector<Trip> &trips, double system_time_s)
{
    auto accumulated_time_s = system_time_s;

    for (const auto &wp : waypoints)
    {
        accumulated_time_s += wp.route.duration_s;

        if (wp.op == WaypointOp::PICKUP && accumulated_time_s > trips[wp.trip_id].max_pickup_time_s)
        {
            return false;
        }
    }

    return true;
}

template <typename RouterFunc>
std::pair<bool, double> get_pickup_time(
    const Trip &trip,
    const Vehicle &vehicle,
    size_t pickup_index,
    double system_time_s,
    RouterFunc &router_func)
{
    double pickup_time_s = system_time_s;

    auto pos = vehicle.pos;
    auto index = 0;
    while (index < pickup_index)
    {
        auto route_response = router_func(pos, vehicle.waypoints[index].pos);

        if (route_response.status != RoutingStatus::OK)
        {
            return {false, 0.0};
        }

        pos = vehicle.waypoints[index++].pos;
        pickup_time_s += route_response.route.duration_s;
    }

    auto route_response = router_func(pos, trip.origin);

    if (route_response.status != RoutingStatus::OK)
    {
        return {false, 0.0};
    }

    return {true, pickup_time_s + route_response.route.duration_s};
}

template <typename RouterFunc>
InsertionResult compute_cost_of_inserting_trip_to_vehicle(
    const Trip &trip,
    const std::vector<Trip> &trips,
    const Vehicle &vehicle,
    double system_time_s,
    RouterFunc &router_func)
{
    InsertionResult ret;

    // Compute the current cost of serving all existing waypoints.
    const auto current_cost_s = get_cost_of_waypoints(vehicle.waypoints);

    const auto num_wps = vehicle.waypoints.size();

    // The pickup and dropoff can be inserted into any position of the current waypoint list.
    for (auto pickup_index = 0; pickup_index <= num_wps; pickup_index++)
    {
        // If we can not pick up the trip before the max wait time time, stop iterating.
        auto [success_pickup, pickup_time_s] = get_pickup_time(trip, vehicle, pickup_index, system_time_s, router_func);
        if (!success_pickup || pickup_time_s > trip.max_pickup_time_s)
        {
            break;
        }

        for (auto dropoff_index = pickup_index; dropoff_index <= num_wps; dropoff_index++)
        {
            auto [success_this_insert, cost_s_this_insert] = compute_cost_of_inserting_trip_to_vehicle(trip, trips, vehicle, pickup_index, dropoff_index, system_time_s, router_func);

            if (success_this_insert && cost_s_this_insert - current_cost_s < ret.cost_s)
            {
                ret.success = true;
                ret.cost_s = cost_s_this_insert - current_cost_s;
                ret.pickup_index = pickup_index;
                ret.dropoff_index = dropoff_index;
            }
        }
    }

    return ret;
}

template <typename RouterFunc>
std::pair<bool, double> compute_cost_of_inserting_trip_to_vehicle(
    const Trip &trip,
    const std::vector<Trip> &trips,
    const Vehicle &vehicle,
    size_t pickup_index,
    size_t dropoff_index,
    double system_time_s,
    RouterFunc &router_func)
{
    auto wps = generate_waypoints(trip, vehicle, pickup_index, dropoff_index, router_func);

    if (wps.empty())
    {
        return {false, 0.0};
    }

    if (!validate_waypoints(wps, trips, system_time_s))
    {
        return {false, 0.0};
    }

    return {true, get_cost_of_waypoints(wps)};
}

template <typename RouterFunc>
void insert_trip_to_vehicle(
    Trip &trip,
    Vehicle &vehicle,
    size_t pickup_index,
    size_t dropoff_index,
    RouterFunc &router_func)
{
    auto wps = generate_waypoints(trip, vehicle, pickup_index, dropoff_index, router_func);

    assert(!wps.empty() && "The generated waypoint list should be never empty!");

    trip.status = TripStatus::DISPATCHED;
    vehicle.waypoints = std::move(wps);

    return;
}

template <typename RouterFunc>
std::vector<Waypoint> generate_waypoints(
    const Trip &trip,
    const Vehicle &vehicle,
    size_t pickup_index,
    size_t dropoff_index,
    RouterFunc &router_func)
{
    std::vector<Waypoint> ret;

    auto pos = vehicle.pos;
    auto index = 0;
    while (true)
    {
        if (index == pickup_index)
        {
            auto route_response = router_func(pos, trip.origin);

            if (route_response.status != RoutingStatus::OK)
            {
                return {};
            }

            pos = trip.origin;
            ret.emplace_back(Waypoint{pos, WaypointOp::PICKUP, trip.id, std::move(route_response.route)});
        }

        if (index == dropoff_index)
        {
            auto route_response = router_func(pos, trip.destination);

            if (route_response.status != RoutingStatus::OK)
            {
                return {};
            }

            pos = trip.destination;
            ret.emplace_back(Waypoint{pos, WaypointOp::DROPOFF, trip.id, std::move(route_response.route)});
        }

        if (index >= vehicle.waypoints.size())
        {
            return ret;
        }

        auto route_response = router_func(pos, vehicle.waypoints[index].pos);

        if (route_response.status != RoutingStatus::OK)
        {
            return {};
        }

        pos = vehicle.waypoints[index].pos;
        ret.emplace_back(Waypoint{pos, vehicle.waypoints[index].op, vehicle.waypoints[index].trip_id, std::move(route_response.route)});

        index++;
    }

    assert(false && "Logical error! We should never reach this line of code!");
}