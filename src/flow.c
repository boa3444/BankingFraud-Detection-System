#include "../include/flow.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   1. FORD-FULKERSON MAX FLOW (BFS AUGMENTING PATHS)
   ============================================================ */

static int bfsAugmentingPath(
    int n,
    int source,
    int sink,
    const double* residual,
    int* parent,
    int* visited,
    int* queue,
    Metrics* metrics
)
{
    memset(visited, 0, (size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) parent[i] = -1;

    int front = 0, back = 0;
    queue[back++] = source;
    visited[source] = 1;

    while (front < back)
    {
        int u = queue[front++];
        for (int v = 0; v < n; v++)
        {
            countComparison(metrics);
            if (!visited[v] && residual[u * n + v] > 1e-9)
            {
                parent[v] = u;
                visited[v] = 1;
                countSwap(metrics);
                if (v == sink) return 1;
                queue[back++] = v;
            }
        }
    }

    return 0;
}

MaxFlowResult fordFulkersonMaxFlow(
    int vertexCount,
    int source,
    int sink,
    const double* capacityMatrix,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    MaxFlowResult res = {0.0, NULL, vertexCount};
    if (vertexCount <= 0 || !capacityMatrix ||
        source < 0 || source >= vertexCount ||
        sink < 0 || sink >= vertexCount || source == sink)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    size_t matBytes = (size_t)vertexCount * (size_t)vertexCount * sizeof(double);
    res.residual = (double*)malloc(matBytes);
    int* parent = (int*)malloc((size_t)vertexCount * sizeof(int));
    int* visited = (int*)malloc((size_t)vertexCount * sizeof(int));
    int* queue = (int*)malloc((size_t)vertexCount * sizeof(int));

    if (!res.residual || !parent || !visited || !queue)
    {
        free(parent); free(visited); free(queue);
        freeMaxFlowResult(&res);
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    memcpy(res.residual, capacityMatrix, matBytes);

    while (bfsAugmentingPath(vertexCount, source, sink, res.residual, parent, visited, queue, metrics))
    {
        double pathFlow = INFINITY;
        for (int v = sink; v != source; v = parent[v])
        {
            int u = parent[v];
            double cap = res.residual[u * vertexCount + v];
            if (cap < pathFlow) pathFlow = cap;
        }

        for (int v = sink; v != source; v = parent[v])
        {
            int u = parent[v];
            res.residual[u * vertexCount + v] -= pathFlow;
            res.residual[v * vertexCount + u] += pathFlow;
            countSwap(metrics);
        }

        res.maxFlow += pathFlow;
    }

    trackBytes(metrics, matBytes + (size_t)vertexCount * sizeof(int) * 3);
    free(parent);
    free(visited);
    free(queue);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeMaxFlowResult(MaxFlowResult* res)
{
    if (!res) return;
    free(res->residual);
    res->residual = NULL;
    res->vertexCount = 0;
}

/* ============================================================
   2. MAXIMUM BIPARTITE MATCHING (DERIVED VIA MAX FLOW)
   ============================================================ */

BipartiteMatchingResult maxBipartiteMatching(
    int leftCount,
    int rightCount,
    const int* adjMatrix,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    BipartiteMatchingResult res = {0, NULL, leftCount};
    if (leftCount <= 0 || rightCount <= 0 || !adjMatrix)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    res.matchForLeft = (int*)malloc((size_t)leftCount * sizeof(int));
    if (!res.matchForLeft)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }
    for (int i = 0; i < leftCount; i++) res.matchForLeft[i] = -1;

    /*
        Build flow network:
        0 = source
        1..leftCount = left vertices
        leftCount+1..leftCount+rightCount = right vertices
        totalV - 1 = sink
    */
    int totalV = leftCount + rightCount + 2;
    int source = 0;
    int sink = totalV - 1;
    double* cap = (double*)calloc((size_t)totalV * (size_t)totalV, sizeof(double));
    if (!cap)
    {
        freeBipartiteMatchingResult(&res);
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int u = 0; u < leftCount; u++)
    {
        cap[source * totalV + (1 + u)] = 1.0;
        for (int v = 0; v < rightCount; v++)
        {
            if (adjMatrix[u * rightCount + v] != 0)
            {
                cap[(1 + u) * totalV + (1 + leftCount + v)] = 1.0;
            }
        }
    }
    for (int v = 0; v < rightCount; v++)
    {
        cap[(1 + leftCount + v) * totalV + sink] = 1.0;
    }

    MaxFlowResult flow = fordFulkersonMaxFlow(totalV, source, sink, cap, metrics);
    res.maxMatches = (int)(flow.maxFlow + 0.5);

    if (flow.residual != NULL)
    {
        for (int u = 0; u < leftCount; u++)
        {
            for (int v = 0; v < rightCount; v++)
            {
                if (adjMatrix[u * rightCount + v] != 0 &&
                    flow.residual[(1 + u) * totalV + (1 + leftCount + v)] < 0.5)
                {
                    res.matchForLeft[u] = v;
                    break;
                }
            }
        }
    }

    freeMaxFlowResult(&flow);
    free(cap);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeBipartiteMatchingResult(BipartiteMatchingResult* res)
{
    if (!res) return;
    free(res->matchForLeft);
    res->matchForLeft = NULL;
    res->leftCount = 0;
}
