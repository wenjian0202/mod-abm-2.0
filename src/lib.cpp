#include <mod/lib.hpp>

#include <fmt/format.h>

#include <tuple>
#include <vector>
#include <algorithm>

std::tuple<double, double> accumulate_vector(const std::vector<double> &values)
{
    fmt::print("Mean: {}, Moment: {}\n", 0.5, 0.6);

    return {0.1, 0.2};
}
