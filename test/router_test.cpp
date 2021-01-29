/// \author Jian Wen
/// \date 2021/01/29

#include "../src/router.hpp"

#include <gtest/gtest.h>

TEST(Router, construct_router_and_find_route)
{
    Router router("../osrm/data/china-latest.osrm");

    Pos origin{113.93593149478123, 22.312648328005512};
    Pos destination{114.13602296340699, 22.28328541732128};

    auto ret = router(origin, destination);
}