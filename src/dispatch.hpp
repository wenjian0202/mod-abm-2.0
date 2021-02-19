/// \author Jian Wen
/// \date 2021/02/02

#pragma once

#include "types.hpp"
#include <cstddef>

/// \brief Assign the pending trips to the vehicles using Insertion Heuristics.
/// \param pending_trip_ids A vector holding indices to the pending trips.
/// \param trips A vector of all trips.
/// \param vehicles A vector of vehicles.
/// \param system_time_s The system time.
/// \tparam router_func The router func that finds path between two poses.
template <typename RouterFunc>
void assign_trips_through_insertion_heuristics(const std::vector<size_t> &pending_trip_ids,
                                               std::vector<Trip> &trips,
                                               std::vector<Vehicle> &vehicles,
                                               double system_time_s,
                                               RouterFunc &router_func);

/// \brief Assign one single trip to the vehicles using using Insertion Heuristics.
/// \param trip The trip to be inserted.
/// \param trips A vector of all trips.
/// \param vehicles A vector of vehicles.
/// \param system_time_s The system time.
/// \tparam router_func The router func that finds path between two poses.
template <typename RouterFunc>
void assign_trip_through_insertion_heuristics(Trip &trip,
                                              const std::vector<Trip> &trips,
                                              std::vector<Vehicle> &vehicles,
                                              RouterFunc &router_func);

/// \brief Compute the cost (time in second) of serving the current waypoints.
/// \details The cost is defined as the total time taken to drop each of the trips based on the
/// current waypoints.
double get_cost_of_waypoints(const std::vector<Waypoint> &waypoints);

/// \brief Validate waypoints by checking the allowed pickup time etc.
bool validate_waypoints(const std::vector<Waypoint> &waypoints,
                        const std::vector<Trip> &trips,
                        const Vehicle &vehicle,
                        double system_time_s);

/// \brief Compute the time that a trip is picked up knowing pickup index.
/// \param pos The current vehicle pose.
/// \param waypoints The waypoints that are orignially planned.
/// \param pickup_pos The pose for the pickup.
/// \param pickup_index The index in the waypoint list where we pick up.
/// \param system_time_s The system time.
/// \tparam router_func The router func that finds path between two poses.
/// \return A pair. True if the trip can be inserted, together with the pick up time. False
/// otherwise.
template <typename RouterFunc>
std::pair<bool, double> get_pickup_time(Pos pos,
                                        const std::vector<Waypoint> &waypoints,
                                        Pos pickup_pos,
                                        size_t pickup_index,
                                        double system_time_s,
                                        RouterFunc &router_func);

/// \brief The return type of the following function.
/// \details If the trip could not be inserted based on the current vehicle status, result is false.
/// Otherwise, result is true. The cost_s is the additional cost in seconds required to serve this
/// trip, The following indices point to where to insert the pickup and dropoff.
struct InsertionResult {
    bool success = false;
    size_t vehicle_id;
    double cost_s = std::numeric_limits<double>::max();
    size_t pickup_index;
    size_t dropoff_index;
};

/// \brief Compute the additional cost (time in second) if a vehicle is to serve a trip.
/// \see get_cost_of_vehicle has the detialed definition of cost.
/// \param trip The trip to be inserted.
/// \param trips A vector of all trips.
/// \param vehicle The vehicle that serves the trip.
/// \param system_time_s The system time.
/// \tparam router_func The router func that finds path between two poses.
template <typename RouterFunc>
InsertionResult compute_cost_of_inserting_trip_to_vehicle(const Trip &trip,
                                                          const std::vector<Trip> &trips,
                                                          const Vehicle &vehicle,
                                                          double system_time_s,
                                                          RouterFunc &router_func);

/// \brief Compute the additional cost knowing pickup and dropoff indices.
/// \param trip The trip to be inserted.
/// \param trips A vector of all trips.
/// \param vehicle The vehicle that serves the trip.
/// \param pickup_index The index in the waypoint list where we pick up.
/// \param dropoff_index The index in the waypoint list where we drop off.
/// \param system_time_s The system time.
/// \tparam router_func The router func that finds path between two poses.
/// \return A pair. True if the trip can be inserted, together with the additional cost in seconds.
/// False otherwise.
template <typename RouterFunc>
std::pair<bool, double> compute_cost_of_inserting_trip_to_vehicle(const Trip &trip,
                                                                  const std::vector<Trip> &trips,
                                                                  const Vehicle &vehicle,
                                                                  size_t pickup_index,
                                                                  size_t dropoff_index,
                                                                  double system_time_s,
                                                                  RouterFunc &router_func);

/// \brief Insert the trip to the vehicle given known pickup and dropoff indices.
/// \param trip The trip to be inserted.
/// \param vehicle The vehicle that serves the trip.
/// \param pickup_index The index in the waypoint list where we pick up.
/// \param dropoff_index The index in the waypoint list where we drop off.
/// \tparam router_func The router func that finds path between two poses.
template <typename RouterFunc>
void insert_trip_to_vehicle(Trip &trip,
                            Vehicle &vehicle,
                            size_t pickup_index,
                            size_t dropoff_index,
                            RouterFunc &router_func);

/// \brief Generate a vector of waypoints given known pickup and dropoff indices.
/// \param trip The trip to be inserted.
/// \param vehicle The vehicle that serves the trip.
/// \param pickup_index The index in the waypoint list where we pick up.
/// \param dropoff_index The index in the waypoint list where we drop off.
/// \param routing_type The type of the route.
/// \tparam router_func The router func that finds path between two poses.
template <typename RouterFunc>
std::vector<Waypoint> generate_waypoints(const Trip &trip,
                                         const Vehicle &vehicle,
                                         size_t pickup_index,
                                         size_t dropoff_index,
                                         RoutingType routing_type,
                                         RouterFunc &router_func);

// Implementation is put in a separate file for clarity and maintainability.
#include "dispatch_impl.hpp"
