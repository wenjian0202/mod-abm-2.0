/// \author Jian Wen
/// \date 2021/02/02

#pragma once

#include "dispatch.hpp"

#include <fmt/format.h>

#include <numeric>

template <typename RouterFunc>
void assign_trips_through_insertion_heuristics(
    std::queue<const std::vector<Trip>::iterator> &trips,
    std::vector<Vehicle> &vehicles,
    RouterFunc &router_func)
{
    fmt::print("[DEBUG] Assigning trips to vehicles through insertion heuristics.\n");

    auto num_trips = trips.size();

    // For each trip, we assign it to the best vehicle.
    for (auto i = 0; i < num_trips; i++)
    {
        auto trip = std::move(trips.front());
        trips.pop();

        auto success = assign_trip_through_insertion_heuristics(trip, vehicles, router_func);

        // If failed to insert the trip, push it back to the queue for future retries.
        if (!success)
        {
            trips.push(std::move(trip));
        }
    }

    return;
}

template <typename RouterFunc>
bool assign_trip_through_insertion_heuristics(
    const std::vector<Trip>::iterator &trip,
    std::vector<Vehicle> &vehicles,
    RouterFunc &router_func)
{
    InsertionResult res;
    size_t best_vehicle_index = 0;

    // Iterate through all vehicles and find the one with least additional cost.
    const auto num_vehs = vehicles.size();
    for (auto vehicle_index = 0; vehicle_index < num_vehs; vehicle_index++)
    {
        auto &vehicle = vehicles[vehicle_index];
        auto res_this_vehicle = compute_cost_of_inserting_trip_to_vehicle(trip, vehicle, router_func);

        if (res_this_vehicle.success && res_this_vehicle.cost_s < res.cost_s)
        {
            res = std::move(res_this_vehicle);
            best_vehicle_index = vehicle_index;
        }
    }

    // If none of the vehicles can serve the trip, return false.
    if (!res.success)
    {
        fmt::print("[DEBUG] Failed to assign trip {}.\n", trip->id);

        return false;
    }

    // Insert the trip to the best vehicle.
    insert_trip_to_vehicle(trip, vehicles[best_vehicle_index], res.pickip_index, res.dropoff_index, router_func);

    fmt::print("[DEBUG] Assigned trip {} to vehicle {}.\n", trip->id, best_vehicle_index);

    return true;
}

template <typename RouterFunc>
InsertionResult compute_cost_of_inserting_trip_to_vehicle(
    const std::vector<Trip>::iterator &trip,
    const Vehicle &vehicle,
    RouterFunc &router_func)
{
    InsertionResult ret;

    // Compute the current total time of serving all existing waypoints.
    const auto current_cost_s = std::accumulate(
        vehicle.waypoints.begin(),
        vehicle.waypoints.end(),
        0.0,
        [](auto val, const auto &wp) { return val + wp.route.duration_s; });

    const auto num_wps = vehicle.waypoints.size();

    // The pickup and dropoff can be inserted into any position of the current waypoint list.
    for (auto pickup_index = 0; pickup_index <= num_wps; pickup_index++)
    {
        for (auto dropoff_index = pickup_index; dropoff_index <= num_wps; dropoff_index++)
        {
            auto [success_this_insert, cost_s_this_insert] = compute_cost_of_inserting_trip_to_vehicle(trip, vehicle, pickup_index, dropoff_index, router_func);

            if (success_this_insert && cost_s_this_insert < ret.cost_s)
            {
                ret.success = true;
                ret.cost_s = cost_s_this_insert;
                ret.pickip_index = pickup_index;
                ret.dropoff_index = dropoff_index;
            }
        }
    }

    return ret;
}

template <typename RouterFunc>
std::pair<bool, double> compute_cost_of_inserting_trip_to_vehicle(
    const std::vector<Trip>::iterator &trip,
    const Vehicle &vehicle,
    size_t pickup_index,
    size_t dropoff_index,
    RouterFunc &router_func)
{
    double cost_s = 0.0;

    auto pos = vehicle.pos;
    auto index = 0;
    while (true)
    {
        if (index == pickup_index)
        {
            auto route_response = router_func(pos, trip->origin);

            if (route_response.status != RoutingStatus::OK)
            {
                return {false, 0.0};
            }

            pos = trip->origin;
            cost_s += route_response.route.duration_s;
        }

        if (index == dropoff_index)
        {
            auto route_response = router_func(pos, trip->destination);

            if (route_response.status != RoutingStatus::OK)
            {
                return {false, 0.0};
            }

            pos = trip->destination;
            cost_s += route_response.route.duration_s;
        }

        if (index >= vehicle.waypoints.size())
        {
            return {true, cost_s};
        }

        auto route_response = router_func(pos, vehicle.waypoints[index].pos);

        if (route_response.status != RoutingStatus::OK)
        {
            return {false, 0.0};
        }

        pos = vehicle.waypoints[index++].pos;
        cost_s += route_response.route.duration_s;
    }

    assert(false && "Logical error in compute_cost_of_inserting_trip_to_vehicle()!");
}

template <typename RouterFunc>
void insert_trip_to_vehicle(
    const std::vector<Trip>::iterator &trip,
    Vehicle &vehicle,
    size_t pickup_index,
    size_t dropoff_index,
    RouterFunc &router_func)
{
    auto pos = vehicle.pos;
    std::vector<Waypoint> wps;

    auto index = 0;
    while (true)
    {
        if (index == pickup_index)
        {
            auto route_response = router_func(pos, trip->origin);

            assert(route_response.status == RoutingStatus::OK && "Logical error in compute_cost_of_inserting_trip_to_vehicle()");

            pos = trip->origin;
            wps.emplace_back(Waypoint{pos, WaypointOp::PICKUP, trip, route_response.route});
        }

        if (index == dropoff_index)
        {
            auto route_response = router_func(pos, trip->destination);

            assert(route_response.status == RoutingStatus::OK && "Logical error in compute_cost_of_inserting_trip_to_vehicle()");

            pos = trip->destination;
            wps.emplace_back(Waypoint{pos, WaypointOp::PICKUP, trip, route_response.route});
        }

        if (index >= vehicle.waypoints.size())
        {
            return;
        }

        auto route_response = router_func(pos, vehicle.waypoints[index].pos);

        assert(route_response.status == RoutingStatus::OK && "Logical error in compute_cost_of_inserting_trip_to_vehicle()");

        pos = vehicle.waypoints[index].pos;
        wps.emplace_back(Waypoint{pos, vehicle.waypoints[index].op, vehicle.waypoints[index].trip, route_response.route});

        index++;
    }

    trip->status = TripStatus::DISPATCHED;
    vehicle.waypoints = std::move(wps);

    return;
}