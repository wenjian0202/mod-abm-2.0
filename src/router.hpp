/// \author Jian Wen
/// \date 2021/01/29

#pragma once

#include "types.hpp"

#include <osrm/osrm.hpp>

#include <string>
#include <memory>

/// \brief Stateful functor that finds the shortest route for an O/D pair on request.
class Router
{
public:
    /// \brief Constructor.
    explicit Router(std::string _path_to_osrm_data);

    /// \brief Main functor that finds the shortest route for an O/D pair on request.
    RoutingResponse operator()(const Pos &origin, const Pos &destination);

private:
    /// \brief The unique pointer to the osrm routing engine instance.
    std::unique_ptr<osrm::OSRM> osrm_ptr_;
};