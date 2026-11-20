#ifndef DP_H
#define DP_H

#include "analysis.h"
#include "graph_types.h"
#include "greedy.h"

/* 1. 0/1 Knapsack */
typedef struct
{
    double maxRisk;
    int* selected;
    int count;
} DpKnapsackResult;

DpKnapsackResult knapsack01DP(
    const CaseItem* cases,
    int count,
    int capacityHours,
    Metrics* metrics
);
void freeDpKnapsackResult(DpKnapsackResult* res);

/* 2. Matrix-Chain Multiplication */
typedef struct
{
    long long minOperations;
    int numMatrices;
} MatrixChainResult;

MatrixChainResult matrixChainOrder(
    const int* dims,
    int dimsCount,
    Metrics* metrics
);

/* 3. Longest Common Subsequence (LCS) */
typedef struct
{
    int length;
    char* sequence;
} LcsResult;

LcsResult longestCommonSubsequence(
    const char* first,
    const char* second,
    Metrics* metrics
);
void freeLcsResult(LcsResult* res);

/* 4. Optimal Binary Search Tree (OBST) */
typedef struct
{
    double minCost;
    int rootIndex;
} ObstResult;

ObstResult optimalBST(
    const double* freq,
    int n,
    Metrics* metrics
);

/* 5. Bellman-Ford Shortest Path (supports negative weights) */
typedef struct
{
    double* distance;
    int* parent;
    int vertexCount;
    int hasNegativeCycle;
} BellmanFordResult;

BellmanFordResult bellmanFord(
    const WeightedGraphData* graph,
    int source,
    Metrics* metrics
);
void freeBellmanFordResult(BellmanFordResult* res);

/* 6. Floyd-Warshall All-Pairs Shortest Paths */
typedef struct
{
    double* dist;
    int vertexCount;
    int hasNegativeCycle;
} FloydWarshallResult;

FloydWarshallResult floydWarshall(
    const WeightedGraphData* graph,
    Metrics* metrics
);
void freeFloydWarshallResult(FloydWarshallResult* res);

/* 7. Bounded Travelling Salesperson (Held-Karp Bitmask DP, n <= 20) */
typedef struct
{
    double minCost;
    int* tour;
    int tourLength;
    int feasible;
} TspResult;

TspResult tspHeldKarp(
    const double* costMatrix,
    int n,
    Metrics* metrics
);
void freeTspResult(TspResult* res);

#endif
