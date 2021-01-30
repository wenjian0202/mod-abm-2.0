/// \author Jian Wen
/// \date 2021/01/29

#include "../src/router.hpp"

#include <benchmark/benchmark.h>

static void BenchmarkRouter(benchmark::State &state)
{
    // Set up the router
    Router router("../osrm/data/china-latest.osrm");

    for (auto _ : state)
    {
        // Time the code
        Pos origin{113.93593149478123, 22.312648328005512};
        Pos destination{114.13602296340699, 22.28328541732128};

        auto ret = router(origin, destination);
    }
}

// Register the function as a benchmark
BENCHMARK(BenchmarkRouter);

// Run the benchmark
BENCHMARK_MAIN();