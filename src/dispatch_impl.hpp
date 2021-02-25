/// \author Jian Wen
/// \date 2021/02/02

#pragma once

#include "dispatch.hpp"

#include <fmt/format.h>

#include <numeric>

template <typename RouterFunc>
void assign_trips_through_insertion_heuristics(const std::vector<size_t> &pending_trip_ids,
                                               std::vector<Trip> &trips,
                                               std::vector<Vehicle> &vehicles,
                                               double system_time_ms,
                                               RouterFunc &router_func) {
    fmt::print("[DEBUG] Assigning trips to vehicles through insertion heuristics.\n");

    // For each trip, we assign it to the best vehicle.
    for (auto trip_id : pending_trip_ids) {
        auto &trip = trips[trip_id];
        assign_trip_through_insertion_heuristics(
            trip, trips, vehicles, system_time_ms, router_func);
    }

    return;
}

template <typename RouterFunc>
void assign_trip_through_insertion_heuristics(Trip &trip,
                                              const std::vector<Trip> &trips,
                                              std::vector<Vehicle> &vehicles,
                                              double system_time_ms,
                                              RouterFunc &router_func) {
    InsertionResult res;

    // Iterate through all vehicles and find the one with least additional cost.
    for (const auto &vehicle : vehicles) {
        auto res_this_vehicle = compute_cost_of_inserting_trip_to_vehicle(
            trip, trips, vehicle, system_time_ms, router_func);

        if (res_this_vehicle.success && res_this_vehicle.cost_ms < res.cost_ms) {
            res = std::move(res_this_vehicle);
        }
    }

    // If none of the vehicles can serve the trip, return false.
    if (!res.success) {
        trip.status = TripStatus::WALKAWAY;
        fmt::print("[DEBUG] Failed to assign Trip #{}.\n", trip.id);

        return;
    }

    // Insert the trip to the best vehicle.
    auto &best_vehicle = vehicles[res.vehicle_id];
    insert_trip_to_vehicle(trip, best_vehicle, res.pickup_index, res.dropoff_index, router_func);

    fmt::print("[DEBUG] Assigned Trip #{} to Vehicle #{}, which has {} waypoints.\n",
               trip.id,
               best_vehicle.id,
               best_vehicle.waypoints.size());

    return;
}

uint64_t get_cost_of_waypoints(const std::vector<Waypoint> &waypoints) {
    auto cost_ms = 0;
    auto accumulated_time_ms = 0;

    for (const auto &wp : waypoints) {
        accumulated_time_ms += wp.route.duration_ms;

        if (wp.op == WaypointOp::DROPOFF) {
            cost_ms += accumulated_time_ms;
        }
    }

    return cost_ms;
}

bool validate_waypoints(const std::vector<Waypoint> &waypoints,
                        const std::vector<Trip> &trips,
                        const Vehicle &vehicle,
                        uint64_t system_time_ms) {
    auto accumulated_time_ms = system_time_ms;
    auto load = vehicle.load;

    for (const auto &wp : waypoints) {
        accumulated_time_ms += wp.route.duration_ms;

        // The planned pickup time should be no larger than the max allowed pickup time.
        if (wp.op == WaypointOp::PICKUP &&
            accumulated_time_ms > trips[wp.trip_id].max_pickup_time_ms) {
            return false;
        }

        if (wp.op == WaypointOp::PICKUP) {
            load++;

            // The load should not exceed the vehicle capacity.
            if (load > vehicle.capacity) {
                return false;
            }
        } else if (wp.op == WaypointOp::DROPOFF) {
            load--;
        }
    }

    return true;
}

template <typename RouterFunc>
std::pair<bool, uint64_t> get_pickup_time(Pos pos,
                                          const std::vector<Waypoint> &waypoints,
                                          Pos pickup_pos,
                                          size_t pickup_index,
                                          uint64_t system_time_ms,
                                          RouterFunc &router_func) {
    auto pickup_time_ms = system_time_ms;

    auto index = 0;
    while (index < pickup_index) {
        pickup_time_ms += waypoints[index].route.duration_ms;
        pos = waypoints[index++].pos;
    }

    auto route_response = router_func(pos, pickup_pos, RoutingType::TIME_ONLY);

    if (route_response.status != RoutingStatus::OK) {
        return {false, 0};
    }

    return {true, pickup_time_ms + route_response.route.duration_ms};
}

template <typename RouterFunc>
InsertionResult compute_cost_of_inserting_trip_to_vehicle(const Trip &trip,
                                                          const std::vector<Trip> &trips,
                                                          const Vehicle &vehicle,
                                                          double system_time_ms,
                                                          RouterFunc &router_func) {
    InsertionResult ret;

    // Compute the current cost of serving all existing waypoints.
    const auto current_cost_ms = get_cost_of_waypoints(vehicle.waypoints);

    const auto num_wps = vehicle.waypoints.size();

    // The pickup and dropoff can be inserted into any position of the current waypoint list.
    for (auto pickup_index = 0; pickup_index <= num_wps; pickup_index++) {
        // If we can not pick up the trip before the max wait time time, stop iterating.
        auto [success_pickup, pickup_time_ms] = get_pickup_time(
            vehicle.pos, vehicle.waypoints, trip.origin, pickup_index, system_time_ms, router_func);

        if (!success_pickup || pickup_time_ms > trip.max_pickup_time_ms) {
            break;
        }

        for (auto dropoff_index = pickup_index; dropoff_index <= num_wps; dropoff_index++) {
            auto [success_this_insert, cost_ms_this_insert] =
                compute_cost_of_inserting_trip_to_vehicle(
                    trip, trips, vehicle, pickup_index, dropoff_index, system_time_ms, router_func);

            if (success_this_insert && cost_ms_this_insert - current_cost_ms < ret.cost_ms) {
                ret.success = true;
                ret.vehicle_id = vehicle.id;
                ret.cost_ms = cost_ms_this_insert - current_cost_ms;
                ret.pickup_index = pickup_index;
                ret.dropoff_index = dropoff_index;
            }
        }
    }

    return ret;
}

template <typename RouterFunc>
std::pair<bool, double> compute_cost_of_inserting_trip_to_vehicle(const Trip &trip,
                                                                  const std::vector<Trip> &trips,
                                                                  const Vehicle &vehicle,
                                                                  size_t pickup_index,
                                                                  size_t dropoff_index,
                                                                  double system_time_ms,
                                                                  RouterFunc &router_func) {
    auto wps = generate_waypoints(
        trip, vehicle, pickup_index, dropoff_index, RoutingType::TIME_ONLY, router_func);

    if (wps.empty()) {
        return {false, 0.0};
    }

    if (!validate_waypoints(wps, trips, vehicle, system_time_ms)) {
        return {false, 0.0};
    }

    return {true, get_cost_of_waypoints(wps)};
}

template <typename RouterFunc>
void insert_trip_to_vehicle(Trip &trip,
                            Vehicle &vehicle,
                            size_t pickup_index,
                            size_t dropoff_index,
                            RouterFunc &router_func) {
    auto wps = generate_waypoints(
        trip, vehicle, pickup_index, dropoff_index, RoutingType::FULL_ROUTE, router_func);

    assert(!wps.empty() && "The generated waypoint list should be never empty!");

    trip.status = TripStatus::DISPATCHED;
    vehicle.waypoints = std::move(wps);

    return;
}

template <typename RouterFunc>
std::vector<Waypoint> generate_waypoints(const Trip &trip,
                                         const Vehicle &vehicle,
                                         size_t pickup_index,
                                         size_t dropoff_index,
                                         RoutingType routing_type,
                                         RouterFunc &router_func) {
    std::vector<Waypoint> ret;

    auto pos = vehicle.pos;
    auto index = 0;
    while (true) {
        if (index == pickup_index) {
            auto route_response = router_func(pos, trip.origin, routing_type);

            if (route_response.status != RoutingStatus::OK) {
                return {};
            }

            pos = trip.origin;
            ret.emplace_back(
                Waypoint{pos, WaypointOp::PICKUP, trip.id, std::move(route_response.route)});
        }

        if (index == dropoff_index) {
            auto route_response = router_func(pos, trip.destination, routing_type);

            if (route_response.status != RoutingStatus::OK) {
                return {};
            }

            pos = trip.destination;
            ret.emplace_back(
                Waypoint{pos, WaypointOp::DROPOFF, trip.id, std::move(route_response.route)});
        }

        if (index >= vehicle.waypoints.size()) {
            return ret;
        }

        auto route_response = router_func(pos, vehicle.waypoints[index].pos, routing_type);

        if (route_response.status != RoutingStatus::OK) {
            return {};
        }

        pos = vehicle.waypoints[index].pos;
        ret.emplace_back(Waypoint{pos,
                                  vehicle.waypoints[index].op,
                                  vehicle.waypoints[index].trip_id,
                                  std::move(route_response.route)});

        index++;
    }

    assert(false && "Logical error! We should never reach this line of code!");
}
