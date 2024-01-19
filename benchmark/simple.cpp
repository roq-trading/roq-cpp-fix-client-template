/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <benchmark/benchmark.h>

#include "roq/tools/simple.hpp"

using namespace roq::tools;

void BM_tools_Simple_add(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(Simple::add(1, 2));
  }
}

BENCHMARK(BM_tools_Simple_add);
