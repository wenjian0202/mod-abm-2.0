/// \author Jian Wen
/// \date 2021/02/08

#include "vehicle.hpp"

#include <fmt/format.h>

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

    step.distance_m *= (1 - ratio);
    step.duration_s *= (1 - ratio);

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

    // Early return.
    if (time_s == 0.0)
    {
        return;
    }

    for (auto i = 0; i < leg.steps.size(); i++)
    {
        auto &step = leg.steps[i];

        // If we can finish this step within the time, remove the entire step.
        if (step.duration_s <= time_s)
        {
            time_s -= step.duration_s;
            continue;
        }

        truncate_step_by_time(step, time_s);
        leg.steps.erase(leg.steps.begin(), leg.steps.begin() + i);

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

    // Early return.
    if (time_s == 0.0)
    {
        return;
    }

    for (auto i = 0; i < route.legs.size(); i++)
    {
        auto &leg = route.legs[i];

        // If we can finish this step within the time, remove the entire step.
        if (leg.duration_s <= time_s)
        {
            time_s -= leg.duration_s;
            continue;
        }

        truncate_leg_by_time(leg, time_s);
        route.legs.erase(route.legs.begin(), route.legs.begin() + i);

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
    // Early return.
    if (time_s == 0.0)
    {
        return;
    }

    for (auto i = 0; i < vehicle.waypoints.size(); i++)
    {
        auto &wp = vehicle.waypoints[i];

        // If we can finish this waypoint within the time.
        if (wp.route.duration_s <= time_s)
        {
            system_time_s += wp.route.duration_s;
            time_s -= wp.route.duration_s;

            vehicle.pos = wp.pos;

            if (wp.op == WaypointOp::PICKUP)
            {
                assert(vehicle.load < vehicle.capacity && "Vehicle's load should never exceed its capacity!");

                trips[wp.trip_id].pickup_time_s = system_time_s;
                vehicle.load++;
            }
            else if (wp.op == WaypointOp::DROPOFF)
            {
                assert(vehicle.load > 0 && "Vehicle's load should not be zero before a dropoff!");

                trips[wp.trip_id].dropoff_time_s = system_time_s;
                vehicle.load--;
            }

            continue;
        }

        // If we can not finish this waypoint, truncate the route.
        truncate_route_by_time(wp.route, time_s);
        vehicle.pos = wp.route.legs.front().steps.front().poses.front();

        vehicle.waypoints.erase(vehicle.waypoints.begin(), vehicle.waypoints.begin() + i);

        return;
    }

    // We've finished all waypoints.
    vehicle.waypoints.clear();
    return;
}
