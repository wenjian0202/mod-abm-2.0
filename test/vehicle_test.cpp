/// \author Jian Wen
/// \date 2021/02/10

#include "../src/vehicle.hpp"

#include <gtest/gtest.h>

TEST(AdvanceStepByTime, return_early_if_time_is_zero)
{
    Step step{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};

    truncate_step_by_time(step, 0.0);

    EXPECT_DOUBLE_EQ(step.distance_m, 10.0);
    EXPECT_DOUBLE_EQ(step.duration_s, 2.0);

    EXPECT_EQ(step.poses.size(), 3);
    EXPECT_DOUBLE_EQ(step.poses[0].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[0].lat, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lat, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[2].lon, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[2].lat, 5.0);
}

TEST(AdvanceStepByTime, return_correct_answer_scenario_1)
{
    Step step{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};

    truncate_step_by_time(step, 0.5);

    EXPECT_DOUBLE_EQ(step.distance_m, 7.5);
    EXPECT_DOUBLE_EQ(step.duration_s, 1.5);

    EXPECT_EQ(step.poses.size(), 3);
    EXPECT_DOUBLE_EQ(step.poses[0].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[0].lat, 2.5);
    EXPECT_DOUBLE_EQ(step.poses[1].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lat, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[2].lon, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[2].lat, 5.0);
}

TEST(AdvanceStepByTime, return_correct_answer_scenario_2)
{
    Step step{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};

    truncate_step_by_time(step, 1.0);

    EXPECT_DOUBLE_EQ(step.distance_m, 5.0);
    EXPECT_DOUBLE_EQ(step.duration_s, 1.0);

    EXPECT_EQ(step.poses.size(), 2);
    EXPECT_DOUBLE_EQ(step.poses[0].lon, 0.0);
    EXPECT_DOUBLE_EQ(step.poses[0].lat, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lon, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lat, 5.0);
}

TEST(AdvanceStepByTime, return_correct_answer_scenario_3)
{
    Step step{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};

    truncate_step_by_time(step, 1.5);

    EXPECT_DOUBLE_EQ(step.distance_m, 2.5);
    EXPECT_DOUBLE_EQ(step.duration_s, 0.5);

    EXPECT_EQ(step.poses.size(), 2);
    EXPECT_DOUBLE_EQ(step.poses[0].lon, 2.5);
    EXPECT_DOUBLE_EQ(step.poses[0].lat, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lon, 5.0);
    EXPECT_DOUBLE_EQ(step.poses[1].lat, 5.0);
}

TEST(AdvanceLegByTime, return_early_if_time_is_zero)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg{20.0, 4.0, {step1, step2}};

    truncate_leg_by_time(leg, 0.0);

    EXPECT_DOUBLE_EQ(leg.distance_m, 20.0);
    EXPECT_DOUBLE_EQ(leg.duration_s, 4.0);

    EXPECT_EQ(leg.steps.size(), 2);
}

TEST(AdvanceLegByTime, return_correct_answer_scenario_1)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg{20.0, 4.0, {step1, step2}};

    truncate_leg_by_time(leg, 1.0);

    EXPECT_DOUBLE_EQ(leg.distance_m, 15.0);
    EXPECT_DOUBLE_EQ(leg.duration_s, 3.0);

    EXPECT_EQ(leg.steps.size(), 2);

    EXPECT_DOUBLE_EQ(leg.steps[0].distance_m, 5.0);
    EXPECT_DOUBLE_EQ(leg.steps[0].duration_s, 1.0);

    EXPECT_DOUBLE_EQ(leg.steps[1].distance_m, 10.0);
    EXPECT_DOUBLE_EQ(leg.steps[1].duration_s, 2.0);
}

TEST(AdvanceLegByTime, return_correct_answer_scenario_2)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg{20.0, 4.0, {step1, step2}};

    truncate_leg_by_time(leg, 2.0);

    EXPECT_DOUBLE_EQ(leg.distance_m, 10.0);
    EXPECT_DOUBLE_EQ(leg.duration_s, 2.0);

    EXPECT_EQ(leg.steps.size(), 1);

    EXPECT_DOUBLE_EQ(leg.steps[0].distance_m, 10.0);
    EXPECT_DOUBLE_EQ(leg.steps[0].duration_s, 2.0);
}

TEST(AdvanceLegByTime, return_correct_answer_scenario_3)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg{20.0, 4.0, {step1, step2}};

    truncate_leg_by_time(leg, 3.0);

    EXPECT_DOUBLE_EQ(leg.distance_m, 5.0);
    EXPECT_DOUBLE_EQ(leg.duration_s, 1.0);

    EXPECT_EQ(leg.steps.size(), 1);

    EXPECT_DOUBLE_EQ(leg.steps[0].distance_m, 5.0);
    EXPECT_DOUBLE_EQ(leg.steps[0].duration_s, 1.0);
}

TEST(AdvanceRouteByTime, return_early_if_time_is_zero)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20.0, 4.0, {step1, step2}};
    Step step3{10.0, 2.0, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10.0, 2.0, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20.0, 4.0, {step3, step4}};
    Route route{40.0, 8.0, {leg1, leg2}};

    truncate_route_by_time(route, 0.0);

    EXPECT_DOUBLE_EQ(route.distance_m, 40.0);
    EXPECT_DOUBLE_EQ(route.duration_s, 8.0);

    EXPECT_EQ(route.legs.size(), 2);
}

TEST(AdvanceRouteByTime, return_correct_answer_scenario_1)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20.0, 4.0, {step1, step2}};
    Step step3{10.0, 2.0, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10.0, 2.0, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20.0, 4.0, {step3, step4}};
    Route route{40.0, 8.0, {leg1, leg2}};

    truncate_route_by_time(route, 2.0);

    EXPECT_DOUBLE_EQ(route.distance_m, 30.0);
    EXPECT_DOUBLE_EQ(route.duration_s, 6.0);

    EXPECT_EQ(route.legs.size(), 2);

    EXPECT_DOUBLE_EQ(route.legs[0].distance_m, 10.0);
    EXPECT_DOUBLE_EQ(route.legs[0].duration_s, 2.0);

    EXPECT_DOUBLE_EQ(route.legs[1].distance_m, 20.0);
    EXPECT_DOUBLE_EQ(route.legs[1].duration_s, 4.0);
}

TEST(AdvanceRouteByTime, return_correct_answer_scenario_2)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20.0, 4.0, {step1, step2}};
    Step step3{10.0, 2.0, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10.0, 2.0, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20.0, 4.0, {step3, step4}};
    Route route{40.0, 8.0, {leg1, leg2}};

    truncate_route_by_time(route, 4.0);

    EXPECT_DOUBLE_EQ(route.distance_m, 20.0);
    EXPECT_DOUBLE_EQ(route.duration_s, 4.0);

    EXPECT_EQ(route.legs.size(), 1);

    EXPECT_DOUBLE_EQ(route.legs[0].distance_m, 20.0);
    EXPECT_DOUBLE_EQ(route.legs[0].duration_s, 4.0);
}

TEST(AdvanceRouteByTime, return_correct_answer_scenario_3)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20.0, 4.0, {step1, step2}};
    Step step3{10.0, 2.0, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10.0, 2.0, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20.0, 4.0, {step3, step4}};
    Route route{40.0, 8.0, {leg1, leg2}};

    truncate_route_by_time(route, 6.0);

    EXPECT_DOUBLE_EQ(route.distance_m, 10.0);
    EXPECT_DOUBLE_EQ(route.duration_s, 2.0);

    EXPECT_EQ(route.legs.size(), 1);

    EXPECT_DOUBLE_EQ(route.legs[0].distance_m, 10.0);
    EXPECT_DOUBLE_EQ(route.legs[0].duration_s, 2.0);
}

TEST(AdvanceVehicleByTime, return_early_if_time_is_zero)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20.0, 4.0, {step1, step2}};
    Step step3{10.0, 2.0, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10.0, 2.0, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20.0, 4.0, {step3, step4}};
    Route route{40.0, 8.0, {leg1, leg2}};

    Waypoint waypoint{Pos{20, 20}, WaypointOp::PICKUP, 0, route};

    Vehicle vehicle{Pos{0, 0}, 2, 0, {waypoint}};

    std::vector<Trip> trips = {Trip{}, Trip{}};

    advance_vehicle(vehicle, trips, 1000.0, 0.0);

    EXPECT_DOUBLE_EQ(vehicle.pos.lon, 0.0);
    EXPECT_DOUBLE_EQ(vehicle.pos.lat, 0.0);

    EXPECT_EQ(vehicle.load, 0);

    EXPECT_EQ(vehicle.waypoints.size(), 1);
    EXPECT_DOUBLE_EQ(vehicle.waypoints[0].route.distance_m, 40.0);
    EXPECT_DOUBLE_EQ(vehicle.waypoints[0].route.duration_s, 8.0);
}

TEST(AdvanceVehicleByTime, not_complete_the_first_waypoint)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20.0, 4.0, {step1, step2}};
    Step step3{10.0, 2.0, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10.0, 2.0, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20.0, 4.0, {step3, step4}};
    Route route{40.0, 8.0, {leg1, leg2}};

    Waypoint waypoint{Pos{20, 20}, WaypointOp::PICKUP, 0, route};

    Vehicle vehicle{Pos{0, 0}, 2, 0, {waypoint}};

    std::vector<Trip> trips = {Trip{}, Trip{}};

    advance_vehicle(vehicle, trips, 1000.0, 4.0);

    EXPECT_DOUBLE_EQ(vehicle.pos.lon, 10.0);
    EXPECT_DOUBLE_EQ(vehicle.pos.lat, 10.0);

    EXPECT_EQ(vehicle.load, 0);

    EXPECT_EQ(vehicle.waypoints.size(), 1);
    EXPECT_DOUBLE_EQ(vehicle.waypoints[0].route.distance_m, 20.0);
    EXPECT_DOUBLE_EQ(vehicle.waypoints[0].route.duration_s, 4.0);
}

TEST(AdvanceVehicleByTime, complete_the_first_waypoint)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20.0, 4.0, {step1, step2}};
    Step step3{10.0, 2.0, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10.0, 2.0, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20.0, 4.0, {step3, step4}};
    Route route{40.0, 8.0, {leg1, leg2}};

    Waypoint waypoint{Pos{20, 20}, WaypointOp::PICKUP, 0, route};

    Vehicle vehicle{Pos{0, 0}, 2, 0, {waypoint}};

    std::vector<Trip> trips = {Trip{}, Trip{}};

    advance_vehicle(vehicle, trips, 1000.0, 10.0);

    EXPECT_DOUBLE_EQ(vehicle.pos.lon, 20.0);
    EXPECT_DOUBLE_EQ(vehicle.pos.lat, 20.0);

    EXPECT_EQ(vehicle.load, 1);

    EXPECT_EQ(vehicle.waypoints.size(), 0);

    EXPECT_DOUBLE_EQ(trips[0].pickup_time_s, 1008.0);
}

TEST(AdvanceVehicleByTime, not_complete_the_second_waypoint)
{
    Step step1{10.0, 2.0, {Pos{0, 0}, Pos{0, 5}, Pos{5, 5}}};
    Step step2{10.0, 2.0, {Pos{5, 5}, Pos{10, 5}, Pos{10, 10}}};
    Leg leg1{20.0, 4.0, {step1, step2}};
    Step step3{10.0, 2.0, {Pos{10, 10}, Pos{10, 15}, Pos{15, 15}}};
    Step step4{10.0, 2.0, {Pos{15, 15}, Pos{20, 15}, Pos{20, 20}}};
    Leg leg2{20.0, 4.0, {step3, step4}};
    Route route{40.0, 8.0, {leg1, leg2}};

    Waypoint waypoint1{Pos{0, 0}, WaypointOp::DROPOFF, 0, Route{40.0, 8.0, {}}};
    Waypoint waypoint2{Pos{20, 20}, WaypointOp::PICKUP, 1, route};

    Vehicle vehicle{Pos{0, 0}, 2, 2, {waypoint1, waypoint2}};

    std::vector<Trip> trips = {Trip{}, Trip{}};

    advance_vehicle(vehicle, trips, 1000.0, 10.0);

    EXPECT_DOUBLE_EQ(vehicle.pos.lon, 5.0);
    EXPECT_DOUBLE_EQ(vehicle.pos.lat, 5.0);

    EXPECT_EQ(vehicle.load, 1);

    EXPECT_EQ(vehicle.waypoints.size(), 1);
    EXPECT_DOUBLE_EQ(vehicle.waypoints[0].route.distance_m, 30.0);
    EXPECT_DOUBLE_EQ(vehicle.waypoints[0].route.duration_s, 6.0);

    EXPECT_DOUBLE_EQ(trips[0].dropoff_time_s, 1008.0);
}
