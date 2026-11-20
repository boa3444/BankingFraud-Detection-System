#ifndef FLOW_H
#define FLOW_H

#include "analysis.h"

/* 1. Ford-Fulkerson (Edmonds-Karp) Maximum Flow */
typedef struct
{
    double maxFlow;
    double* residual;
    int vertexCount;
} MaxFlowResult;

MaxFlowResult fordFulkersonMaxFlow(
    int vertexCount,
    int source,
    int sink,
    const double* capacityMatrix,
    Metrics* metrics
);
void freeMaxFlowResult(MaxFlowResult* res);

/* 2. Maximum Bipartite Matching (e.g., Fraud Cases <-> Investigators) */
typedef struct
{
    int maxMatches;
    int* matchForLeft;
    int leftCount;
} BipartiteMatchingResult;

BipartiteMatchingResult maxBipartiteMatching(
    int leftCount,
    int rightCount,
    const int* adjMatrix,
    Metrics* metrics
);
void freeBipartiteMatchingResult(BipartiteMatchingResult* res);

#endif
