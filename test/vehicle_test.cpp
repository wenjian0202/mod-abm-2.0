/// \author Jian Wen
/// \date 2021/02/10

#include "../src/vehicle.hpp"

#include <gtest/gtest.h>

TEST(AdvanceStepByTime, return_early_if_time_is_zero) {
    Step step{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};

    truncate_step_by_time(step, 0);

    EXPECT_EQ(step.distance_mm, 10000);
    EXPECT_EQ(step.duration_ms, 2000);

    EXPECT_EQ(step.poses.size(), 3);
    EXPECT_DOUBLE_EQ(step.poses[0].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[0].lat, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lat, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[2].lon, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[2].lat, 5.0);
}

TEST(AdvanceStepByTime, return_correct_answer_scenario_1) {
    Step step{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};

    truncate_step_by_time(step, 500);

    EXPECT_EQ(step.distance_mm, 7500);
    EXPECT_EQ(step.duration_ms, 1500);

    EXPECT_EQ(step.poses.size(), 3);
    EXPECT_DOUBLE_EQ(step.poses[0].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[0].lat, 2.5);
    EXPECT_DOUBLE_EQ(step.poses[1].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lat, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[2].lon, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[2].lat, 5.0);
}

TEST(AdvanceStepByTime, return_correct_answer_scenario_2) {
    Step step{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};

    truncate_step_by_time(step, 1000);

    EXPECT_EQ(step.distance_mm, 5000);
    EXPECT_EQ(step.duration_ms, 1000);

    EXPECT_EQ(step.poses.size(), 2);
    EXPECT_DOUBLE_EQ(step.poses[0].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[0].lat, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lon, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lat, 5.0);
}

TEST(AdvanceStepByTime, return_correct_answer_scenario_3) {
    Step step{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};

    truncate_step_by_time(step, 1500);

    EXPECT_EQ(step.distance_mm, 2500);
    EXPECT_EQ(step.duration_ms, 500);

    EXPECT_EQ(step.poses.size(), 2);
    EXPECT_DOUBLE_EQ(step.poses[0].lon, 2.5);
    EXPECT_DOUBLE_EQ(step.poses[0].lat, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lon, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lat, 5.0);
}

TEST(AdvanceLegByTime, return_early_if_time_is_zero) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg{20000, 4000, {step1, step2}};

    truncate_leg_by_time(leg, 0);

    EXPECT_EQ(leg.distance_mm, 20000);
    EXPECT_EQ(leg.duration_ms, 4000);

    EXPECT_EQ(leg.steps.size(), 2);
}

TEST(AdvanceLegByTime, return_correct_answer_scenario_1) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg{20000, 4000, {step1, step2}};

    truncate_leg_by_time(leg, 1000);

    EXPECT_EQ(leg.distance_mm, 15000);
    EXPECT_EQ(leg.duration_ms, 3000);

    EXPECT_EQ(leg.steps.size(), 2);

    EXPECT_EQ(leg.steps[0].distance_mm, 5000);
    EXPECT_EQ(leg.steps[0].duration_ms, 1000);

    EXPECT_EQ(leg.steps[1].distance_mm, 10000);
    EXPECT_EQ(leg.steps[1].duration_ms, 2000);
}

TEST(AdvanceLegByTime, return_correct_answer_scenario_2) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg{20000, 4000, {step1, step2}};

    truncate_leg_by_time(leg, 2000);

    EXPECT_EQ(leg.distance_mm, 10000);
    EXPECT_EQ(leg.duration_ms, 2000);

    EXPECT_EQ(leg.steps.size(), 1);

    EXPECT_EQ(leg.steps[0].distance_mm, 10000);
    EXPECT_EQ(leg.steps[0].duration_ms, 2000);
}

TEST(AdvanceLegByTime, return_correct_answer_scenario_3) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg{20000, 4000, {step1, step2}};

    truncate_leg_by_time(leg, 3000);

    EXPECT_EQ(leg.distance_mm, 5000);
    EXPECT_EQ(leg.duration_ms, 1000);

    EXPECT_EQ(leg.steps.size(), 1);

    EXPECT_EQ(leg.steps[0].distance_mm, 5000);
    EXPECT_EQ(leg.steps[0].duration_ms, 1000);
}

TEST(AdvanceRouteByTime, return_early_if_time_is_zero) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20000, 4000, {step1, step2}};
    Step step3{10000, 2000, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10000, 2000, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20000, 4000, {step3, step4}};
    Route route{40000, 8000, {leg1, leg2}};

    truncate_route_by_time(route, 0);

    EXPECT_EQ(route.distance_mm, 40000);
    EXPECT_EQ(route.duration_ms, 8000);

    EXPECT_EQ(route.legs.size(), 2);
}

TEST(AdvanceRouteByTime, return_correct_answer_scenario_1) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20000, 4000, {step1, step2}};
    Step step3{10000, 2000, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10000, 2000, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20000, 4000, {step3, step4}};
    Route route{40000, 8000, {leg1, leg2}};

    truncate_route_by_time(route, 2000);

    EXPECT_EQ(route.distance_mm, 30000);
    EXPECT_EQ(route.duration_ms, 6000);

    EXPECT_EQ(route.legs.size(), 2);

    EXPECT_EQ(route.legs[0].distance_mm, 10000);
    EXPECT_EQ(route.legs[0].duration_ms, 2000);

    EXPECT_EQ(route.legs[1].distance_mm, 20000);
    EXPECT_EQ(route.legs[1].duration_ms, 4000);
}

TEST(AdvanceRouteByTime, return_correct_answer_scenario_2) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20000, 4000, {step1, step2}};
    Step step3{10000, 2000, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10000, 2000, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20000, 4000, {step3, step4}};
    Route route{40000, 8000, {leg1, leg2}};

    truncate_route_by_time(route, 4000);

    EXPECT_EQ(route.distance_mm, 20000);
    EXPECT_EQ(route.duration_ms, 4000);

    EXPECT_EQ(route.legs.size(), 1);

    EXPECT_EQ(route.legs[0].distance_mm, 20000);
    EXPECT_EQ(route.legs[0].duration_ms, 4000);
}

TEST(AdvanceRouteByTime, return_correct_answer_scenario_3) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20000, 4000, {step1, step2}};
    Step step3{10000, 2000, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10000, 2000, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20000, 4000, {step3, step4}};
    Route route{40000, 8000, {leg1, leg2}};

    truncate_route_by_time(route, 6000);

    EXPECT_EQ(route.distance_mm, 10000);
    EXPECT_EQ(route.duration_ms, 2000);

    EXPECT_EQ(route.legs.size(), 1);

    EXPECT_EQ(route.legs[0].distance_mm, 10000);
    EXPECT_EQ(route.legs[0].duration_ms, 2000);
}

TEST(AdvanceVehicleByTime, return_early_if_time_is_zero) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20000, 4000, {step1, step2}};
    Step step3{10000, 2000, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10000, 2000, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20000, 4000, {step3, step4}};
    Route route{40000, 8000, {leg1, leg2}};

    Waypoint waypoint{Pos{20, 20}, WaypointOp::PICKUP, 0, route};

    Vehicle vehicle{0, Pos{0, 0}, 2, 0, {waypoint}, 0, 0};

    std::vector<Trip> trips = {Trip{}, Trip{}};

    advance_vehicle(vehicle, trips, 1000000, 0);

    EXPECT_DOUBLE_EQ(vehicle.pos.lon, 0.0);
    EXPECT_DOUBLE_EQ(vehicle.pos.lat, 0.0);

    EXPECT_EQ(vehicle.load, 0);

    EXPECT_EQ(vehicle.waypoints.size(), 1);
    EXPECT_EQ(vehicle.waypoints[0].route.distance_mm, 40000);
    EXPECT_EQ(vehicle.waypoints[0].route.duration_ms, 8000);

    EXPECT_EQ(vehicle.dist_traveled_mm, 0000);
    EXPECT_EQ(vehicle.loaded_dist_traveled_mm, 0000);
}

TEST(AdvanceVehicleByTime, not_complete_the_first_waypoint) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20000, 4000, {step1, step2}};
    Step step3{10000, 2000, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10000, 2000, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20000, 4000, {step3, step4}};
    Route route{40000, 8000, {leg1, leg2}};

    Waypoint waypoint{Pos{20, 20}, WaypointOp::PICKUP, 0, route};

    Vehicle vehicle{0, Pos{0, 0}, 2, 1, {waypoint}, 0, 0};

    std::vector<Trip> trips = {Trip{}, Trip{}};

    advance_vehicle(vehicle, trips, 1000000, 4000);

    EXPECT_DOUBLE_EQ(vehicle.pos.lon, 10.0);
    EXPECT_DOUBLE_EQ(vehicle.pos.lat, 10.0);

    EXPECT_EQ(vehicle.load, 1);

    EXPECT_EQ(vehicle.waypoints.size(), 1);
    EXPECT_EQ(vehicle.waypoints[0].route.distance_mm, 20000);
    EXPECT_EQ(vehicle.waypoints[0].route.duration_ms, 4000);

    EXPECT_EQ(vehicle.dist_traveled_mm, 20000);
    EXPECT_EQ(vehicle.loaded_dist_traveled_mm, 20000);
}

TEST(AdvanceVehicleByTime, complete_the_first_waypoint) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20000, 4000, {step1, step2}};
    Step step3{10000, 2000, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10000, 2000, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20000, 4000, {step3, step4}};
    Route route{40000, 8000, {leg1, leg2}};

    Waypoint waypoint{Pos{20, 20}, WaypointOp::PICKUP, 0, route};

    Vehicle vehicle{0, Pos{0, 0}, 2, 1, {waypoint}, 0, 0};

    std::vector<Trip> trips = {Trip{}, Trip{}};

    advance_vehicle(vehicle, trips, 1000000, 10000);

    EXPECT_DOUBLE_EQ(vehicle.pos.lon, 20.0);
    EXPECT_DOUBLE_EQ(vehicle.pos.lat, 20.0);

    EXPECT_EQ(vehicle.load, 2);

    EXPECT_EQ(vehicle.waypoints.size(), 0);

    EXPECT_EQ(vehicle.dist_traveled_mm, 40000);
    EXPECT_EQ(vehicle.loaded_dist_traveled_mm, 40000);

    EXPECT_EQ(trips[0].pickup_time_ms, 1008000);
}

TEST(AdvanceVehicleByTime, not_complete_the_second_waypoint) {
    Step step1{10000, 2000, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10000, 2000, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20000, 4000, {step1, step2}};
    Step step3{10000, 2000, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10000, 2000, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20000, 4000, {step3, step4}};
    Route route{40000, 8000, {leg1, leg2}};

    Waypoint waypoint1{Pos{0, 0}, WaypointOp::DROPOFF, 0, Route{40000, 8000, {}}};
    Waypoint waypoint2{Pos{20, 20}, WaypointOp::PICKUP, 1, route};

    Vehicle vehicle{0, Pos{0, 0}, 2, 2, {waypoint1, waypoint2}, 0, 0};

    std::vector<Trip> trips = {Trip{}, Trip{}};

    advance_vehicle(vehicle, trips, 1000000, 10000);

    EXPECT_DOUBLE_EQ(vehicle.pos.lon, 5.0);
    EXPECT_DOUBLE_EQ(vehicle.pos.lat, 5.0);

    EXPECT_EQ(vehicle.load, 1);

    EXPECT_EQ(vehicle.waypoints.size(), 1);
    EXPECT_EQ(vehicle.waypoints[0].route.distance_mm, 30000);
    EXPECT_EQ(vehicle.waypoints[0].route.duration_ms, 6000);

    EXPECT_EQ(vehicle.dist_traveled_mm, 50000);
    EXPECT_EQ(vehicle.loaded_dist_traveled_mm, 90000);

    EXPECT_EQ(trips[0].dropoff_time_ms, 1008000);
}
