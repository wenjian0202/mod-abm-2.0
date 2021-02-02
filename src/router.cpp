/// \author Jian Wen
/// \date 2021/01/29

#include "router.hpp"

#include <osrm/route_parameters.hpp>
#include <osrm/engine_config.hpp>
#include <osrm/json_container.hpp>

#include <fmt/format.h>

Router::Router(std::string _path_to_osrm_data)
{
    // Set up the OSRM backend routing engine
    osrm::EngineConfig config;

    // Path to the base osrm map data
    config.storage_config = {std::move(_path_to_osrm_data)};

    // No shared memory
    config.use_shared_memory = false;

    // Use Multi-Level Dijkstra (MLD) for routing. This requires extract+partition+customize pre-processing.
    config.algorithm = osrm::EngineConfig::Algorithm::MLD;

    // Create the routing engine instance
    osrm_ptr_ = std::make_unique<osrm::OSRM>(config);

    fmt::print("[INFO] Initiated the OSRM routing engine using map data from {}.\n", _path_to_osrm_data);
}

RoutingResponse Router::operator()(const Pos &origin, const Pos &destination)
{
    // Convert to the osrm route request params
    osrm::RouteParameters params;

    // Origin -> Destination
    params.coordinates.push_back({osrm::util::FloatLongitude{origin.lon}, osrm::util::FloatLatitude{origin.lat}});
    params.coordinates.push_back({osrm::util::FloatLongitude{destination.lon}, osrm::util::FloatLatitude{destination.lat}});

    // Set up other params
    params.steps = true;                                                // returns the detailed steps of the route
    params.alternatives = false;                                        // no alternative routes, just find the best one
    params.geometries = osrm::RouteParameters::GeometriesType::GeoJSON; // route geometry encoded in GeoJSON
    params.overview = osrm::RouteParameters::OverviewType::False;       // no route overview

    // Response is in JSON format
    osrm::engine::api::ResultT result = osrm::json::Object();

    // Execute routing request, which does the heavy lifting
    const auto status = osrm_ptr_->Route(params, result);

    // Parse the result
    auto &json_result = result.get<osrm::json::Object>();
    RoutingResponse response;

    if (status == osrm::Status::Ok)
    {
        auto &routes = json_result.values["routes"].get<osrm::json::Array>();

        // Return empty response if empty route
        if (routes.values.empty())
        {
            response.status = RoutingStatus::EMPTY;
            response.message = "No routes returned between the requested origin and destination.";

            return response;
        }

        // Let's just use the first route
        auto &route = routes.values.at(0).get<osrm::json::Object>();
        const auto distance = route.values["distance"].get<osrm::json::Number>().value;
        const auto duration = route.values["duration"].get<osrm::json::Number>().value;

        // Return empty response if extract does not contain the default coordinates from above
        if (distance == 0 || duration == 0)
        {
            response.status = RoutingStatus::EMPTY;
            response.message = "Distance or duration of route is zero. You are probably doing a query outside of the OSM extract.";

            return response;
        }

        response.status = RoutingStatus::OK;
        response.route = convert_json_to_route(std::move(route));

        return response;
    }

    const auto code = json_result.values["code"].get<osrm::json::String>().value;
    const auto message = json_result.values["message"].get<osrm::json::String>().value;

    response.status = RoutingStatus::ERROR;
    response.message = fmt::format("Code: {}, Message {}", code, message);

    return response;
}