/// \author Jian Wen
/// \date 2021/02/01

#pragma once

#include "types.hpp"

#include <string>
#include <memory>

/// \brief Stateful functor that generates trips based on demand data.
class DemandGenerator
{
public:
    /// \brief Constructor.
    explicit DemandGenerator(std::string _path_to_demand_data);

    /// \brief Main functor that generates the requests until the target system time.
    std::vector<Request> operator()(double target_system_time_s);

private:
    /// \brief Generate a request following the Poisson process and trip intensities.
    /// \see the definition of OdWithProb for detailed explaination.
    Request generate_request(double last_request_time_s);

    /// \brief The time of the last request.
    Request last_request_ = {};

    /// \brief The system time starting from 0.0.
    double system_time_s_ = 0.0;

    /// \brief The demand ODs and their accumulated probabilities.
    /// \see the definition of OdWithProb for detailed explaination.
    std::vector<OdWithProb> ods_ = {};

    /// \brief The time of the last request.
    double trips_per_hour_ = 0.0;
};