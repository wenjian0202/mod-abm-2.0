/// \author Jian Wen
/// \date 2021/02/02

#pragma once

#include "types.hpp"

#include <queue>

/// \brief Assign the pending trips to the vehicles using Insertion Heuristics.
/// \param trips A vector of iterators to the pending trips.
/// \param vehicles A vector of vehicles.
/// \tparam router_func The router func that finds path between two poses.
template <typename RouterFunc>
void assign_trips_through_insertion_heuristics(
    std::queue<const std::vector<Trip>::iterator> &trips,
    std::vector<Vehicle> &vehicles,
    RouterFunc &router_func);

/// \brief Assign one single trip to the vehicles using using Insertion Heuristics.
/// \param trip An iterator to the trip to be inserted.
/// \param vehicles A vector of vehicles.
/// \tparam router_func The router func that finds path between two poses.
template <typename RouterFunc>
bool assign_trip_through_insertion_heuristics(
    const std::vector<Trip>::iterator &trip,
    std::vector<Vehicle> &vehicles,
    RouterFunc &router_func);

/// \brief The return type of the following function.
/// \details If the trip could not be inserted based on the current vehicle status, result is false.
/// Otherwise, result is true. The cost_s is the additional time in seconds required to serve this trip,
/// The following indices point to where to insert the pickup and dropoff.
struct InsertionResult
{
    bool success = false;
    double cost_s = std::numeric_limits<double>::max();
    size_t pickip_index;
    size_t dropoff_index;
};

/// \brief Compute the additional cost (time in second) if a vehicle is to serve a trip.
/// \param trip An iterator to the trip to be inserted.
/// \param vehicle The vehicle that serves the trip.
/// \tparam router_func The router func that finds path between two poses.
template <typename RouterFunc>
InsertionResult compute_cost_of_inserting_trip_to_vehicle(
    const std::vector<Trip>::iterator &trip,
    const Vehicle &vehicle,
    RouterFunc &router_func);

/// \brief Compute the additional cost knowing pickup and dropoff indices.
/// \param trip An iterator to the trip to be inserted.
/// \param vehicle The vehicle that serves the trip.
/// \param pickup_index The index in the waypoint list where we pick up.
/// \param dropoff_index The index in the waypoint list where we drop off.
/// \tparam router_func The router func that finds path between two poses.
/// \return A pair. True if the trip can be inserted, together with the additional cost in seconds. False otherwise.
template <typename RouterFunc>
std::pair<bool, double> compute_cost_of_inserting_trip_to_vehicle(
    const std::vector<Trip>::iterator &trip,
    const Vehicle &vehicle,
    size_t pickup_index,
    size_t dropoff_index,
    RouterFunc &router_func);

/// \brief Insert the trip to the vehicle given known pickup and dropoff indices.
/// \param trip An iterator to the trip to be inserted.
/// \param vehicle The vehicle that serves the trip.
/// \param pickup_index The index in the waypoint list where we pick up.
/// \param dropoff_index The index in the waypoint list where we drop off.
/// \tparam router_func The router func that finds path between two poses.
template <typename RouterFunc>
void insert_trip_to_vehicle(
    const std::vector<Trip>::iterator &trip,
    Vehicle &vehicle,
    size_t pickup_index,
    size_t dropoff_index,
    RouterFunc &router_func);

// Implementation is put in a separate file for clarity and maintainability.
#include "dispatch_impl.hpp"
