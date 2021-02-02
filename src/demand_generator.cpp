/// \author Jian Wen
/// \date 2021/02/01

#include "demand_generator.hpp"

#include <fmt/format.h>

#include <algorithm>

DemandGenerator::DemandGenerator(std::string _path_to_demand_data)
{
    auto demand_yaml = YAML::LoadFile(_path_to_demand_data);

    for (const auto &od_yaml : demand_yaml)
    {
        trips_per_hour_ += od_yaml["trips_per_hour"].as<double>();
    }

    auto accumucated_trips = 0.0;

    for (const auto &od_yaml : demand_yaml)
    {
        OdWithProb od;

        od.origin.lon = od_yaml["origin"]["lon"].as<double>();
        od.origin.lat = od_yaml["origin"]["lat"].as<double>();
        od.destination.lon = od_yaml["destination"]["lon"].as<double>();
        od.destination.lat = od_yaml["destination"]["lat"].as<double>();

        accumucated_trips += od_yaml["trips_per_hour"].as<double>();
        od.accumulated_prob = accumucated_trips / trips_per_hour_;

        ods_.emplace_back(std::move(od));
    }

    fmt::print("[INFO] Loaded demand config from {}. Generated demand matrix with {} OD pairs and {} total trips per hour.\n", _path_to_demand_data, ods_.size(), trips_per_hour_);
}

std::vector<Request> DemandGenerator::operator()(double target_system_time_s)
{
    assert(system_time_s_ <= target_system_time_s && "[ERROR] The target_system_time should be no less than the current system time in Demand Generator!");

    // System time moves forward by a cycle.
    system_time_s_ = target_system_time_s;

    std::vector<Request> requests;

    // Get the last request time. 0.0 indicates the initial state with no requests generated yet.
    if (last_request_.request_time_s > 0.0)
    {
        if (last_request_.request_time_s > system_time_s_)
        {
            fmt::print("[DEBUG] T = {}: Generated {} request(s) in this cycle:\n", system_time_s_, requests.size());

            return requests;
        }

        requests.emplace_back(last_request_);
    }

    // Generate new requests until we go beyond this cycle.
    while (true)
    {
        last_request_ = generate_request(last_request_.request_time_s);

        if (last_request_.request_time_s > system_time_s_)
        {
            break;
        }

        requests.emplace_back(last_request_);
    }

    fmt::print("[DEBUG] T = {}: Generated {} request(s) in this cycle:\n", system_time_s_, requests.size());
    for (const auto &request : requests)
    {
        fmt::print("[DEBUG]  Request at time {}, from ({}, {}) to ({}, {}):\n", request.request_time_s, request.origin.lon, request.origin.lat, request.destination.lon, request.destination.lat);
    }

    return requests;
}

Request DemandGenerator::generate_request(double last_request_time_s)
{
    // Generate a random number in [0, 1)
    auto rn = rand() / static_cast<double>(RAND_MAX);

    // Based on the accumulated probabilities of each OD, find the corresponding one
    auto od_it = std::lower_bound(ods_.begin(), ods_.end(), rn, [](OdWithProb od, double val) { return od.accumulated_prob < val; });

    // Calculate thg interarrival time that follows the Poisson process
    auto interval_s = -log(1 - rand() / static_cast<double>(RAND_MAX)) / trips_per_hour_ * 3600;

    return {od_it->origin, od_it->destination, last_request_time_s + interval_s};
}