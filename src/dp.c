#include "../include/dp.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   1. 0/1 KNAPSACK (INTEGER HOURS)
   ============================================================ */

DpKnapsackResult knapsack01DP(
    const CaseItem* cases,
    int count,
    int capacityHours,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    DpKnapsackResult res = {0.0, NULL, 0};
    if (cases == NULL || count <= 0 || capacityHours <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int cols = capacityHours + 1;
    double* dp = (double*)calloc((size_t)(count + 1) * (size_t)cols, sizeof(double));
    res.selected = (int*)calloc((size_t)count, sizeof(int));
    if (!dp || !res.selected)
    {
        free(dp);
        free(res.selected);
        res.selected = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int i = 1; i <= count; i++)
    {
        int w = (int)(cases[i - 1].hoursRequired + 0.5);
        double v = cases[i - 1].riskScore;
        for (int cap = 0; cap <= capacityHours; cap++)
        {
            double skip = dp[(i - 1) * cols + cap];
            double best = skip;
            if (w > 0 && w <= cap && v >= 0.0)
            {
                double take = dp[(i - 1) * cols + (cap - w)] + v;
                countComparison(metrics);
                if (take > best) best = take;
            }
            dp[i * cols + cap] = best;
            countSwap(metrics);
        }
    }

    res.maxRisk = dp[count * cols + capacityHours];
    res.count = count;

    int rem = capacityHours;
    for (int i = count; i >= 1; i--)
    {
        int w = (int)(cases[i - 1].hoursRequired + 0.5);
        if (dp[i * cols + rem] != dp[(i - 1) * cols + rem] && w > 0 && w <= rem)
        {
            res.selected[i - 1] = 1;
            rem -= w;
        }
    }

    trackBytes(metrics, (size_t)(count + 1) * (size_t)cols * sizeof(double));
    free(dp);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeDpKnapsackResult(DpKnapsackResult* res)
{
    if (!res) return;
    free(res->selected);
    res->selected = NULL;
    res->count = 0;
}

/* ============================================================
   2. MATRIX-CHAIN MULTIPLICATION
   ============================================================ */

MatrixChainResult matrixChainOrder(
    const int* dims,
    int dimsCount,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    MatrixChainResult res = {0, 0};
    if (dims == NULL || dimsCount < 2)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int n = dimsCount - 1;
    res.numMatrices = n;
    if (n <= 1)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    long long* m = (long long*)calloc((size_t)n * (size_t)n, sizeof(long long));
    if (!m)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int chainLen = 2; chainLen <= n; chainLen++)
    {
        for (int i = 0; i <= n - chainLen; i++)
        {
            int j = i + chainLen - 1;
            m[i * n + j] = LLONG_MAX;
            for (int k = i; k < j; k++)
            {
                long long cost = m[i * n + k] + m[(k + 1) * n + j] +
                    (long long)dims[i] * (long long)dims[k + 1] * (long long)dims[j + 1];
                countComparison(metrics);
                if (cost < m[i * n + j])
                {
                    m[i * n + j] = cost;
                    countSwap(metrics);
                }
            }
        }
    }

    res.minOperations = m[0 * n + (n - 1)];
    trackBytes(metrics, (size_t)n * (size_t)n * sizeof(long long));
    free(m);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

/* ============================================================
   3. LONGEST COMMON SUBSEQUENCE (LCS)
   ============================================================ */

LcsResult longestCommonSubsequence(
    const char* first,
    const char* second,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    LcsResult res = {0, NULL};
    if (!first || !second)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int n = (int)strlen(first);
    int m = (int)strlen(second);
    int cols = m + 1;
    int* dp = (int*)calloc((size_t)(n + 1) * (size_t)cols, sizeof(int));
    if (!dp)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            countComparison(metrics);
            if (first[i - 1] == second[j - 1])
            {
                dp[i * cols + j] = dp[(i - 1) * cols + (j - 1)] + 1;
            }
            else
            {
                int up = dp[(i - 1) * cols + j];
                int left = dp[i * cols + (j - 1)];
                dp[i * cols + j] = (up >= left) ? up : left;
            }
            countSwap(metrics);
        }
    }

    int len = dp[n * cols + m];
    res.length = len;
    res.sequence = (char*)malloc((size_t)len + 1);
    if (res.sequence)
    {
        res.sequence[len] = '\0';
        int i = n, j = m, pos = len - 1;
        while (i > 0 && j > 0)
        {
            if (first[i - 1] == second[j - 1])
            {
                res.sequence[pos--] = first[i - 1];
                i--;
                j--;
            }
            else if (dp[(i - 1) * cols + j] >= dp[i * cols + (j - 1)])
            {
                i--;
            }
            else
            {
                j--;
            }
        }
    }

    trackBytes(metrics, (size_t)(n + 1) * (size_t)cols * sizeof(int));
    free(dp);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeLcsResult(LcsResult* res)
{
    if (!res) return;
    free(res->sequence);
    res->sequence = NULL;
    res->length = 0;
}

/* ============================================================
   4. OPTIMAL BINARY SEARCH TREE (OBST)
   ============================================================ */

ObstResult optimalBST(
    const double* freq,
    int n,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    ObstResult res = {0.0, -1};
    if (!freq || n <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    double* cost = (double*)calloc((size_t)n * (size_t)n, sizeof(double));
    int* root = (int*)calloc((size_t)n * (size_t)n, sizeof(int));
    double* prefix = (double*)calloc((size_t)(n + 1), sizeof(double));
    if (!cost || !root || !prefix)
    {
        free(cost); free(root); free(prefix);
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int i = 0; i < n; i++)
    {
        prefix[i + 1] = prefix[i] + freq[i];
        cost[i * n + i] = freq[i];
        root[i * n + i] = i;
    }

    for (int len = 2; len <= n; len++)
    {
        for (int i = 0; i <= n - len; i++)
        {
            int j = i + len - 1;
            double sumFreq = prefix[j + 1] - prefix[i];
            cost[i * n + j] = INFINITY;

            for (int r = i; r <= j; r++)
            {
                double leftCost = (r > i) ? cost[i * n + (r - 1)] : 0.0;
                double rightCost = (r < j) ? cost[(r + 1) * n + j] : 0.0;
                double cand = leftCost + rightCost + sumFreq;
                countComparison(metrics);
                if (cand < cost[i * n + j])
                {
                    cost[i * n + j] = cand;
                    root[i * n + j] = r;
                    countSwap(metrics);
                }
            }
        }
    }

    res.minCost = cost[0 * n + (n - 1)];
    res.rootIndex = root[0 * n + (n - 1)];

    trackBytes(metrics, (size_t)n * (size_t)n * (sizeof(double) + sizeof(int)));
    free(cost);
    free(root);
    free(prefix);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

/* ============================================================
   5. BELLMAN-FORD SHORTEST PATH
   ============================================================ */

BellmanFordResult bellmanFord(
    const WeightedGraphData* graph,
    int source,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    BellmanFordResult res = {NULL, NULL, 0, 0};
    if (!graph || graph->accounts.count <= 0 ||
        source < 0 || source >= graph->accounts.count)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int n = graph->accounts.count;
    res.vertexCount = n;
    res.distance = (double*)malloc((size_t)n * sizeof(double));
    res.parent = (int*)malloc((size_t)n * sizeof(int));
    if (!res.distance || !res.parent)
    {
        freeBellmanFordResult(&res);
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int i = 0; i < n; i++)
    {
        res.distance[i] = INFINITY;
        res.parent[i] = -1;
    }
    res.distance[source] = 0.0;

    for (int iter = 0; iter < n - 1; iter++)
    {
        int updated = 0;
        for (int e = 0; e < graph->edgeCount; e++)
        {
            const WeightedEdge* edge = &graph->directedEdges[e];
            if (!isfinite(res.distance[edge->from])) continue;
            countComparison(metrics);
            double cand = res.distance[edge->from] + edge->weight;
            if (cand < res.distance[edge->to])
            {
                res.distance[edge->to] = cand;
                res.parent[edge->to] = edge->from;
                updated = 1;
                countSwap(metrics);
            }
        }
        if (!updated) break;
    }

    for (int e = 0; e < graph->edgeCount; e++)
    {
        const WeightedEdge* edge = &graph->directedEdges[e];
        if (!isfinite(res.distance[edge->from])) continue;
        countComparison(metrics);
        if (res.distance[edge->from] + edge->weight < res.distance[edge->to])
        {
            res.hasNegativeCycle = 1;
            break;
        }
    }

    trackBytes(metrics, (size_t)n * (sizeof(double) + sizeof(int)));
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeBellmanFordResult(BellmanFordResult* res)
{
    if (!res) return;
    free(res->distance);
    free(res->parent);
    res->distance = NULL;
    res->parent = NULL;
    res->vertexCount = 0;
}

/* ============================================================
   6. FLOYD-WARSHALL ALL-PAIRS SHORTEST PATHS
   ============================================================ */

FloydWarshallResult floydWarshall(
    const WeightedGraphData* graph,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    FloydWarshallResult res = {NULL, 0, 0};
    if (!graph || graph->accounts.count <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int n = graph->accounts.count;
    res.vertexCount = n;
    res.dist = (double*)malloc((size_t)n * (size_t)n * sizeof(double));
    if (!res.dist)
    {
        res.vertexCount = 0;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            res.dist[i * n + j] = (i == j) ? 0.0 : INFINITY;
        }
    }

    for (int e = 0; e < graph->edgeCount; e++)
    {
        const WeightedEdge* edge = &graph->directedEdges[e];
        if (edge->weight < res.dist[edge->from * n + edge->to])
        {
            res.dist[edge->from * n + edge->to] = edge->weight;
        }
    }

    for (int k = 0; k < n; k++)
    {
        for (int i = 0; i < n; i++)
        {
            if (!isfinite(res.dist[i * n + k])) continue;
            for (int j = 0; j < n; j++)
            {
                if (!isfinite(res.dist[k * n + j])) continue;
                countComparison(metrics);
                double cand = res.dist[i * n + k] + res.dist[k * n + j];
                if (cand < res.dist[i * n + j])
                {
                    res.dist[i * n + j] = cand;
                    countSwap(metrics);
                }
            }
        }
    }

    for (int i = 0; i < n; i++)
    {
        if (res.dist[i * n + i] < 0.0)
        {
            res.hasNegativeCycle = 1;
            break;
        }
    }

    trackBytes(metrics, (size_t)n * (size_t)n * sizeof(double));
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeFloydWarshallResult(FloydWarshallResult* res)
{
    if (!res) return;
    free(res->dist);
    res->dist = NULL;
    res->vertexCount = 0;
}

/* ============================================================
   7. BOUNDED TSP (HELD-KARP BITMASK DP)
   ============================================================ */

TspResult tspHeldKarp(
    const double* costMatrix,
    int n,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    TspResult res = {INFINITY, NULL, 0, 0};
    if (!costMatrix || n <= 0 || n > 20)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    if (n == 1)
    {
        res.minCost = 0.0;
        res.feasible = 1;
        res.tourLength = 2;
        res.tour = (int*)calloc(2, sizeof(int));
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int totalMasks = 1 << n;
    double* dp = (double*)malloc((size_t)totalMasks * (size_t)n * sizeof(double));
    int* parent = (int*)malloc((size_t)totalMasks * (size_t)n * sizeof(int));
    if (!dp || !parent)
    {
        free(dp); free(parent);
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int i = 0; i < totalMasks * n; i++)
    {
        dp[i] = INFINITY;
        parent[i] = -1;
    }
    dp[1 * n + 0] = 0.0;

    for (int mask = 1; mask < totalMasks; mask++)
    {
        if (!(mask & 1)) continue;
        for (int u = 0; u < n; u++)
        {
            if (!(mask & (1 << u)) || !isfinite(dp[mask * n + u])) continue;
            for (int v = 0; v < n; v++)
            {
                if (mask & (1 << v)) continue;
                double w = costMatrix[u * n + v];
                if (!isfinite(w)) continue;
                int nextMask = mask | (1 << v);
                double cand = dp[mask * n + u] + w;
                countComparison(metrics);
                if (cand < dp[nextMask * n + v])
                {
                    dp[nextMask * n + v] = cand;
                    parent[nextMask * n + v] = u;
                    countSwap(metrics);
                }
            }
        }
    }

    int fullMask = totalMasks - 1;
    int bestLast = -1;
    for (int u = 1; u < n; u++)
    {
        if (!isfinite(dp[fullMask * n + u]) || !isfinite(costMatrix[u * n + 0])) continue;
        double total = dp[fullMask * n + u] + costMatrix[u * n + 0];
        countComparison(metrics);
        if (total < res.minCost)
        {
            res.minCost = total;
            bestLast = u;
        }
    }

    if (bestLast != -1)
    {
        res.feasible = 1;
        res.tourLength = n + 1;
        res.tour = (int*)malloc((size_t)(n + 1) * sizeof(int));
        if (res.tour)
        {
            int mask = fullMask;
            int cur = bestLast;
            res.tour[n] = 0;
            for (int pos = n - 1; pos >= 0; pos--)
            {
                res.tour[pos] = cur;
                int prev = parent[mask * n + cur];
                mask ^= (1 << cur);
                cur = prev;
            }
        }
    }

    trackBytes(metrics, (size_t)totalMasks * (size_t)n * (sizeof(double) + sizeof(int)));
    free(dp);
    free(parent);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeTspResult(TspResult* res)
{
    if (!res) return;
    free(res->tour);
    res->tour = NULL;
    res->tourLength = 0;
}
