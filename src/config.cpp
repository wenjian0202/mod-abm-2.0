/// \author Jian Wen
/// \date 2021/02/01

#include "config.hpp"

#include <fmt/format.h>

PlatformConfig load_platform_config(const std::string &path_to_platform_config)
{
    auto platform_config_yaml = YAML::LoadFile(path_to_platform_config);

    PlatformConfig platform_config;

    platform_config.area_config.lon_min = platform_config_yaml["area_config"]["lon_min"].as<double>();
    platform_config.area_config.lon_max = platform_config_yaml["area_config"]["lon_max"].as<double>();
    platform_config.area_config.lat_min = platform_config_yaml["area_config"]["lat_min"].as<double>();
    platform_config.area_config.lat_max = platform_config_yaml["area_config"]["lat_max"].as<double>();

    platform_config.mod_system_config.fleet_config.fleet_size = platform_config_yaml["mod_system_config"]["fleet_config"]["fleet_size"].as<size_t>();
    platform_config.mod_system_config.fleet_config.veh_capacity = platform_config_yaml["mod_system_config"]["fleet_config"]["veh_capacity"].as<size_t>();
    platform_config.mod_system_config.fleet_config.initial_lon = platform_config_yaml["mod_system_config"]["fleet_config"]["initial_lon"].as<double>();
    platform_config.mod_system_config.fleet_config.initial_lat = platform_config_yaml["mod_system_config"]["fleet_config"]["initial_lat"].as<double>();

    platform_config.mod_system_config.request_config.max_dispatch_wait_time_s = platform_config_yaml["mod_system_config"]["request_config"]["max_dispatch_wait_time_s"].as<double>();
    platform_config.mod_system_config.request_config.max_pickup_wait_time_s = platform_config_yaml["mod_system_config"]["request_config"]["max_pickup_wait_time_s"].as<double>();

    platform_config.simulation_config.cycle_s = platform_config_yaml["simulation_config"]["cycle_s"].as<double>();
    platform_config.simulation_config.simulation_duration_s = platform_config_yaml["simulation_config"]["simulation_duration_s"].as<double>();
    platform_config.simulation_config.warmup_duration_s = platform_config_yaml["simulation_config"]["warmup_duration_s"].as<double>();
    platform_config.simulation_config.winddown_duration_s = platform_config_yaml["simulation_config"]["winddown_duration_s"].as<double>();

    platform_config.output_config.datalog_config.output_datalog = platform_config_yaml["output_config"]["datalog_config"]["output_datalog"].as<bool>();
    platform_config.output_config.datalog_config.path_to_output_datalog = platform_config_yaml["output_config"]["datalog_config"]["path_to_output_datalog"].as<std::string>();
    platform_config.output_config.video_config.render_video = platform_config_yaml["output_config"]["video_config"]["render_video"].as<bool>();
    platform_config.output_config.video_config.path_to_output_video = platform_config_yaml["output_config"]["video_config"]["path_to_output_video"].as<std::string>();
    platform_config.output_config.video_config.frames_per_cycle = platform_config_yaml["output_config"]["video_config"]["frames_per_cycle"].as<size_t>();
    platform_config.output_config.video_config.replay_speed = platform_config_yaml["output_config"]["video_config"]["replay_speed"].as<double>();

    fmt::print("[INFO] Loaded the platform configuration yaml file from {}.\n", path_to_platform_config);

    // Sanity check of the input config.
    if (platform_config.output_config.datalog_config.output_datalog)
    {
        assert(platform_config.output_config.datalog_config.path_to_output_datalog != "" && "Config must have non-empty path_to_output_datalog if output_datalog is true!");
    }
    if (platform_config.output_config.video_config.render_video)
    {
        assert(platform_config.output_config.datalog_config.output_datalog && "Config must have output_datalog config on if render_video is true!");
        assert(platform_config.output_config.video_config.path_to_output_video != "" && "Config must have non-empty path_to_output_video if render_video is true!");
        assert(platform_config.output_config.video_config.frames_per_cycle > 0 && "Config must have positive frames_per_cycle if render_video is true!");
        assert(platform_config.output_config.video_config.replay_speed > 0 && "Config must have positive frames_per_cycle if render_video is true!");
    }

    return platform_config;
}