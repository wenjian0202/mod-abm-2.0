/// \author Jian Wen
/// \date 2021/01/29

#pragma once

#include <memory>
#include <osrm/osrm.hpp>
#include <string>

#include "types.hpp"

/// \brief Stateful functor that finds the shortest route for an O/D pair on request.
class Router {
  public:
    /// \brief Constructor.
    explicit Router(std::string _path_to_osrm_data);

    /// \brief Main functor that finds the shortest route for an O/D pair on request.
    RoutingResponse operator()(const Pos &origin, const Pos &destination, RoutingType type);

  private:
    /// \brief The unique pointer to the osrm routing engine instance.
    std::unique_ptr<osrm::OSRM> osrm_ptr_;
};

/// \brief Convert the json route data into the c++ data struct.
Route convert_json_to_route(osrm::json::Object route_json);
