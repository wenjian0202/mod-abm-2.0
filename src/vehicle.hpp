/// \author Jian Wen
/// \date 2021/02/08

#pragma once

#include "types.hpp"

/// \brief Trucate Step so that the first x seconds worth of route is completed.
void truncate_step_by_time(Step &step, double time_s);

/// \brief Trucate Leg so that the first x seconds worth of route is completed.
void truncate_leg_by_time(Leg &leg, double time_s);

/// \brief Trucate Route so that the first x seconds worth of route is completed.
void truncate_route_by_time(Route &route, double time_s);

/// \brief Advance the vehicle by x second so that the first x seconds worth of route is completed.
/// \param vehicle the vehicle that contains waypoints to be processed.
/// \param trips the reference to the trips. Completing a waypoint might result in change of trip status.
/// \param system_time_s the current system time in seconds.
/// \param time_s the time in seconds that we need to advance the system.
void advance_vehicle(
    Vehicle &vehicle,
    std::vector<Trip> &trips,
    double system_time_s,
    double time_s);
