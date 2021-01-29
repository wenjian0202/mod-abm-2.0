/// \author Jian Wen
/// \date 2021/01/29

#pragma once

/// \brief Position encoded in longitude/latitude.
/// \details lon in [-180, 180), lat in [-90, 90]. Undefined behaviors if out of bound.
struct Pos
{
    double lon = 0.0;
    double lat = 0.0;
};