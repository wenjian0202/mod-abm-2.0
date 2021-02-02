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
