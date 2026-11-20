#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "analysis.h"
#include "divide_conquer.h"
#include "graph_types.h"
#include "greedy.h"
#include "searching.h"
#include "sorting.h"
#include "string_matching.h"
#include "transaction.h"

/*
    Runs Merge Sort and Quick Sort on the same transaction dataset.
*/
void runSortingBenchmark(
    const Transaction* transactions,
    int count
);

/*
    Runs all four string-matching algorithms on the combined
    transaction descriptions.
*/
void runStringMatchingBenchmark(
    const Transaction* transactions,
    int count,
    const char* pattern
);

/*
    Runs Small, Medium, Large, and Adversarial benchmarks across
    Sorting, Searching, String Matching, Divide & Conquer, and Greedy
    modules, and exports results to CSV.
*/
void runScalabilityBenchmarkSuite(
    const char* csvOutputPath
);

/*
    Runs the complete benchmarking suite.
*/
void runCompleteBenchmark(
    const Transaction* transactions,
    int count
);

#endif
