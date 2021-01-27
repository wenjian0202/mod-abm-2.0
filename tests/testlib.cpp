#include <mod/lib.hpp>

#include <gtest/gtest.h>

TEST(Dummy, Dummy)
{
    std::vector<double> values{1, 2., 3.};
    auto [mean, moment] = accumulate_vector(values);

    ASSERT_EQ(mean, 2.0);
    ASSERT_EQ(moment, 3.0);
}