#ifndef BACKTRACKING_H
#define BACKTRACKING_H

#include "analysis.h"
#include "greedy.h"

/* 1. N-Queens */
typedef struct
{
    int solutionCount;
    int* firstBoard;
    int n;
} NQueensResult;

NQueensResult solveNQueens(int n, Metrics* metrics);
void freeNQueensResult(NQueensResult* res);

/* 2. Sum of Subsets */
typedef struct
{
    int found;
    int solutionCount;
    int* firstSubset;
    int count;
} SubsetSumResult;

SubsetSumResult sumOfSubsets(
    const double* values,
    int count,
    double targetSum,
    Metrics* metrics
);
void freeSubsetSumResult(SubsetSumResult* res);

/* 3. 0/1 Knapsack (Backtracking + FIFO/LIFO/Least-Cost Branch & Bound) */
typedef enum
{
    BNB_FIFO = 0,
    BNB_LIFO = 1,
    BNB_LEAST_COST = 2
} BnBMode;

typedef struct
{
    double maxRisk;
    int* selected;
    int count;
    long long nodesExplored;
} ExactKnapsackResult;

ExactKnapsackResult knapsack01Backtracking(
    const CaseItem* cases,
    int count,
    double capacityHours,
    Metrics* metrics
);

ExactKnapsackResult knapsack01BranchAndBound(
    const CaseItem* cases,
    int count,
    double capacityHours,
    BnBMode mode,
    Metrics* metrics
);

void freeExactKnapsackResult(ExactKnapsackResult* res);

#endif
