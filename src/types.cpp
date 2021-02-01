/// \author Jian Wen
/// \date 2021/02/01

#include "types.hpp"

#include <fmt/format.h>

Route convert_json_to_route(osrm::json::Object route_json)
{
    Route route;

    route.distance_m = route_json.values["distance"].get<osrm::json::Number>().value;
    route.duration_s = route_json.values["duration"].get<osrm::json::Number>().value;

    auto &legs_json = route_json.values["legs"].get<osrm::json::Array>();

    for (auto &leg_json : legs_json.values)
    {
        auto &leg_json_obejct = leg_json.get<osrm::json::Object>();

        Leg leg;
        leg.distance_m = leg_json_obejct.values["distance"].get<osrm::json::Number>().value;
        leg.duration_s = leg_json_obejct.values["duration"].get<osrm::json::Number>().value;

        auto &steps_json = leg_json_obejct.values["steps"].get<osrm::json::Array>();

        for (auto &step_json : steps_json.values)
        {
            auto &step_json_obejct = step_json.get<osrm::json::Object>();

            Step step;
            step.distance_m = step_json_obejct.values["distance"].get<osrm::json::Number>().value;
            step.duration_s = step_json_obejct.values["duration"].get<osrm::json::Number>().value;

            auto &poses_json = step_json_obejct.values["geometry"].get<osrm::json::Object>().values["coordinates"].get<osrm::json::Array>();

            for (auto &pos_json : poses_json.values)
            {
                auto &pos_json_obejct = pos_json.get<osrm::json::Array>();

                Pos pos;
                pos.lon = pos_json_obejct.values[0].get<osrm::json::Number>().value;
                pos.lat = pos_json_obejct.values[1].get<osrm::json::Number>().value;

                step.poses.emplace_back(std::move(pos));
            }

            leg.steps.emplace_back(std::move(step));
        }

        route.legs.emplace_back(std::move(leg));
    }

    return route;
}

PlatformConfig convert_yaml_to_platform_config(const YAML::Node &platform_config_yaml)
{
    PlatformConfig platform_config;

    platform_config.area_config.lon_min = platform_config_yaml["area_config"]["lon_min"].as<double>();
    platform_config.area_config.lon_max = platform_config_yaml["area_config"]["lon_max"].as<double>();
    platform_config.area_config.lat_min = platform_config_yaml["area_config"]["lat_min"].as<double>();
    platform_config.area_config.lat_max = platform_config_yaml["area_config"]["lat_max"].as<double>();

    platform_config.mod_system_config.fleet_config.fleet_size = platform_config_yaml["mod_system_config"]["fleet_config"]["fleet_size"].as<size_t>();
    platform_config.mod_system_config.fleet_config.veh_capacity = platform_config_yaml["mod_system_config"]["fleet_config"]["veh_capacity"].as<size_t>();

    platform_config.mod_system_config.request_config.walkaway_time_s = platform_config_yaml["mod_system_config"]["request_config"]["walkaway_time_s"].as<double>();

    platform_config.simulation_config.cycle_s = platform_config_yaml["simulation_config"]["cycle_s"].as<double>();
    platform_config.simulation_config.simulation_duration_s = platform_config_yaml["simulation_config"]["simulation_duration_s"].as<double>();
    platform_config.simulation_config.warmup_duration_s = platform_config_yaml["simulation_config"]["warmup_duration_s"].as<double>();
    platform_config.simulation_config.winddown_duration_s = platform_config_yaml["simulation_config"]["winddown_duration_s"].as<double>();

    platform_config.path_to_osrm_data = platform_config_yaml["path_to_osrm_data"].as<std::string>();

    return platform_config;
}