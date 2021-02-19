/// \author Jian Wen
/// \date 2021/01/29

#include "../src/types.hpp"

#include <gtest/gtest.h>

TEST(RoutingResponse, default_initialization) {
    RoutingResponse response;

    EXPECT_EQ(response.status, RoutingStatus::UNDEFINED);
}

TEST(Trip, default_initialization) {
    Trip trip;

    EXPECT_EQ(trip.status, TripStatus::UNDEFINED);
}

TEST(Vehicle, default_initialization) {
    Vehicle vehicle;

    EXPECT_EQ(vehicle.capacity, 1);
    EXPECT_EQ(vehicle.load, 0);
    EXPECT_TRUE(vehicle.waypoints.empty());
}
