#include <mod/types.hpp>

#include <gtest/gtest.h>

TEST(Dummy, Dummy)
{
    Pos pos;
    EXPECT_DOUBLE_EQ(pos.lat, 0.0);
}