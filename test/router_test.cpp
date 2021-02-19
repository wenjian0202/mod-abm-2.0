/// \author Jian Wen
/// \date 2021/01/29

#include "../src/router.hpp"

#include <gtest/gtest.h>

TEST(Router, construct_router_and_route_time_only) {
    Router router("../osrm/map/hongkong.osrm");

    Pos origin{114.16490186070844, 22.304400695672847};     // Hong Kong West Kowloon Station
    Pos destination{114.13598336133562, 22.28344162014816}; // The University of Hong Kong

    auto ret = router(origin, destination, RoutingType::TIME_ONLY);

    EXPECT_EQ(ret.status, RoutingStatus::OK);
    EXPECT_EQ(ret.message, "");

    EXPECT_TRUE(ret.route.distance_m > 0);
    EXPECT_TRUE(ret.route.duration_s > 0);
    EXPECT_TRUE(ret.route.legs.empty());
}

TEST(Router, construct_router_and_route_full_route) {
    Router router("../osrm/map/hongkong.osrm");

    Pos origin{114.16490186070844, 22.304400695672847};     // Hong Kong West Kowloon Station
    Pos destination{114.13598336133562, 22.28344162014816}; // The University of Hong Kong

    auto ret = router(origin, destination, RoutingType::FULL_ROUTE);

    EXPECT_EQ(ret.status, RoutingStatus::OK);
    EXPECT_EQ(ret.message, "");

    EXPECT_TRUE(ret.route.distance_m > 0);
    EXPECT_TRUE(ret.route.duration_s > 0);
    EXPECT_FALSE(ret.route.legs.empty());
}

TEST(Router, construct_router_and_route_with_invalid_origin) {
    Router router("../osrm/map/hongkong.osrm");

    Pos origin{360, 100};                                   // Bad origin input
    Pos destination{114.13598336133562, 22.28344162014816}; // The University of Hong Kong

    auto ret = router(origin, destination, RoutingType::FULL_ROUTE);

    EXPECT_EQ(ret.status, RoutingStatus::ERROR);
    EXPECT_EQ(ret.message, "Code: InvalidValue, Message Invalid coordinate value.");
}
