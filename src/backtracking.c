#include "../include/backtracking.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   1. N-QUEENS (BACKTRACKING)
   ============================================================ */

static void nQueensHelper(
    int row,
    int n,
    int* board,
    NQueensResult* res,
    Metrics* metrics
)
{
    countCall(metrics);
    if (row == n)
    {
        if (res->solutionCount == 0 && res->firstBoard != NULL)
        {
            memcpy(res->firstBoard, board, (size_t)n * sizeof(int));
        }
        res->solutionCount++;
        return;
    }

    for (int col = 0; col < n; col++)
    {
        int safe = 1;
        for (int r = 0; r < row; r++)
        {
            countComparison(metrics);
            int c = board[r];
            if (c == col || abs(c - col) == abs(r - row))
            {
                safe = 0;
                break;
            }
        }
        if (safe)
        {
            board[row] = col;
            countSwap(metrics);
            nQueensHelper(row + 1, n, board, res, metrics);
        }
    }
}

NQueensResult solveNQueens(int n, Metrics* metrics)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    NQueensResult res = {0, NULL, n};
    if (n <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int* board = (int*)calloc((size_t)n, sizeof(int));
    res.firstBoard = (int*)calloc((size_t)n, sizeof(int));
    if (!board || !res.firstBoard)
    {
        free(board);
        free(res.firstBoard);
        res.firstBoard = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    nQueensHelper(0, n, board, &res, metrics);
    trackBytes(metrics, (size_t)n * sizeof(int) * 2);
    free(board);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeNQueensResult(NQueensResult* res)
{
    if (!res) return;
    free(res->firstBoard);
    res->firstBoard = NULL;
    res->n = 0;
}

/* ============================================================
   2. SUM OF SUBSETS (BACKTRACKING)
   ============================================================ */

static void subsetSumHelper(
    const double* values,
    const double* suffixSum,
    int count,
    int idx,
    double currentSum,
    double targetSum,
    int* currentMask,
    SubsetSumResult* res,
    Metrics* metrics
)
{
    countCall(metrics);
    if (fabs(currentSum - targetSum) < 1e-6)
    {
        if (!res->found && res->firstSubset != NULL)
        {
            memcpy(res->firstSubset, currentMask, (size_t)count * sizeof(int));
            res->found = 1;
        }
        res->solutionCount++;
        return;
    }

    if (idx >= count) return;
    countComparison(metrics);
    if (currentSum > targetSum + 1e-6) return;
    if (currentSum + suffixSum[idx] < targetSum - 1e-6) return;

    /* Include values[idx] */
    currentMask[idx] = 1;
    countSwap(metrics);
    subsetSumHelper(values, suffixSum, count, idx + 1,
                    currentSum + values[idx], targetSum, currentMask, res, metrics);

    /* Exclude values[idx] */
    currentMask[idx] = 0;
    subsetSumHelper(values, suffixSum, count, idx + 1,
                    currentSum, targetSum, currentMask, res, metrics);
}

SubsetSumResult sumOfSubsets(
    const double* values,
    int count,
    double targetSum,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    SubsetSumResult res = {0, 0, NULL, count};
    if (!values || count <= 0 || targetSum < 0.0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    double* suffix = (double*)calloc((size_t)(count + 1), sizeof(double));
    int* mask = (int*)calloc((size_t)count, sizeof(int));
    res.firstSubset = (int*)calloc((size_t)count, sizeof(int));
    if (!suffix || !mask || !res.firstSubset)
    {
        free(suffix); free(mask); free(res.firstSubset);
        res.firstSubset = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int i = count - 1; i >= 0; i--)
    {
        double v = (values[i] > 0.0) ? values[i] : 0.0;
        suffix[i] = suffix[i + 1] + v;
    }

    subsetSumHelper(values, suffix, count, 0, 0.0, targetSum, mask, &res, metrics);
    trackBytes(metrics, (size_t)count * (sizeof(double) + sizeof(int) * 2));
    free(suffix);
    free(mask);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeSubsetSumResult(SubsetSumResult* res)
{
    if (!res) return;
    free(res->firstSubset);
    res->firstSubset = NULL;
    res->count = 0;
}

/* ============================================================
   3. 0/1 KNAPSACK (BACKTRACKING & BRANCH AND BOUND)
   ============================================================ */

typedef struct
{
    int originalIndex;
    CaseItem item;
} SortedCase;

static void sortCasesDesc(SortedCase* arr, int count)
{
    /* Simple insertion sort for clarity & zero stdlib qsort */
    for (int i = 1; i < count; i++)
    {
        SortedCase key = arr[i];
        double keyRatio = key.item.riskScore / key.item.hoursRequired;
        int j = i - 1;
        while (j >= 0)
        {
            double jr = arr[j].item.riskScore / arr[j].item.hoursRequired;
            if (jr >= keyRatio) break;
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

static double fractionalUpperBound(
    const SortedCase* items,
    int count,
    int nextIdx,
    double currentRisk,
    double currentHours,
    double capacityHours
)
{
    if (currentHours > capacityHours) return -1.0;
    double bound = currentRisk;
    double totalH = currentHours;
    int i = nextIdx;
    while (i < count && totalH + items[i].item.hoursRequired <= capacityHours)
    {
        totalH += items[i].item.hoursRequired;
        bound += items[i].item.riskScore;
        i++;
    }
    if (i < count)
    {
        bound += (capacityHours - totalH) * (items[i].item.riskScore / items[i].item.hoursRequired);
    }
    return bound;
}

static void knapsackBtHelper(
    const SortedCase* items,
    int count,
    int idx,
    double curRisk,
    double curHours,
    double capacityHours,
    int* curMask,
    ExactKnapsackResult* res,
    Metrics* metrics
)
{
    countCall(metrics);
    res->nodesExplored++;

    if (curRisk > res->maxRisk)
    {
        res->maxRisk = curRisk;
        for (int i = 0; i < count; i++)
        {
            res->selected[items[i].originalIndex] = curMask[i];
        }
    }

    if (idx >= count) return;

    double ub = fractionalUpperBound(items, count, idx, curRisk, curHours, capacityHours);
    countComparison(metrics);
    if (ub <= res->maxRisk) return;

    if (curHours + items[idx].item.hoursRequired <= capacityHours)
    {
        curMask[idx] = 1;
        countSwap(metrics);
        knapsackBtHelper(items, count, idx + 1,
                         curRisk + items[idx].item.riskScore,
                         curHours + items[idx].item.hoursRequired,
                         capacityHours, curMask, res, metrics);
        curMask[idx] = 0;
    }

    knapsackBtHelper(items, count, idx + 1,
                     curRisk, curHours, capacityHours, curMask, res, metrics);
}

ExactKnapsackResult knapsack01Backtracking(
    const CaseItem* cases,
    int count,
    double capacityHours,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    ExactKnapsackResult res = {0.0, NULL, count, 0};
    if (!cases || count <= 0 || capacityHours <= 0.0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    SortedCase* items = (SortedCase*)malloc((size_t)count * sizeof(SortedCase));
    int* mask = (int*)calloc((size_t)count, sizeof(int));
    res.selected = (int*)calloc((size_t)count, sizeof(int));
    if (!items || !mask || !res.selected)
    {
        free(items); free(mask); free(res.selected);
        res.selected = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int validCount = 0;
    for (int i = 0; i < count; i++)
    {
        if (cases[i].hoursRequired > 0.0 && cases[i].riskScore >= 0.0)
        {
            items[validCount].originalIndex = i;
            items[validCount].item = cases[i];
            validCount++;
        }
    }
    sortCasesDesc(items, validCount);

    knapsackBtHelper(items, validCount, 0, 0.0, 0.0, capacityHours, mask, &res, metrics);
    trackBytes(metrics, (size_t)count * (sizeof(SortedCase) + sizeof(int) * 2));
    free(items);
    free(mask);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

typedef struct
{
    int level;
    double risk;
    double hours;
    double bound;
    int* mask;
} BnBNode;

static void heapPushBnB(BnBNode* heap, int* size, BnBNode node, Metrics* metrics)
{
    int idx = (*size)++;
    heap[idx] = node;
    while (idx > 0)
    {
        int p = (idx - 1) / 2;
        countComparison(metrics);
        if (heap[idx].bound > heap[p].bound)
        {
            BnBNode t = heap[idx]; heap[idx] = heap[p]; heap[p] = t;
            countSwap(metrics);
            idx = p;
        }
        else break;
    }
}

static BnBNode heapPopBnB(BnBNode* heap, int* size, Metrics* metrics)
{
    BnBNode top = heap[0];
    heap[0] = heap[--(*size)];
    int idx = 0;
    while (1)
    {
        int l = 2 * idx + 1, r = 2 * idx + 2, best = idx;
        if (l < *size) { countComparison(metrics); if (heap[l].bound > heap[best].bound) best = l; }
        if (r < *size) { countComparison(metrics); if (heap[r].bound > heap[best].bound) best = r; }
        if (best != idx)
        {
            BnBNode t = heap[idx]; heap[idx] = heap[best]; heap[best] = t;
            countSwap(metrics);
            idx = best;
        }
        else break;
    }
    return top;
}

ExactKnapsackResult knapsack01BranchAndBound(
    const CaseItem* cases,
    int count,
    double capacityHours,
    BnBMode mode,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    ExactKnapsackResult res = {0.0, NULL, count, 0};
    if (!cases || count <= 0 || capacityHours <= 0.0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    SortedCase* items = (SortedCase*)malloc((size_t)count * sizeof(SortedCase));
    res.selected = (int*)calloc((size_t)count, sizeof(int));
    if (!items || !res.selected)
    {
        free(items); free(res.selected);
        res.selected = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int validCount = 0;
    for (int i = 0; i < count; i++)
    {
        if (cases[i].hoursRequired > 0.0 && cases[i].riskScore >= 0.0)
        {
            items[validCount].originalIndex = i;
            items[validCount].item = cases[i];
            validCount++;
        }
    }
    sortCasesDesc(items, validCount);

    int cap = 1024;
    BnBNode* container = (BnBNode*)malloc((size_t)cap * sizeof(BnBNode));
    if (!container)
    {
        free(items); free(res.selected);
        res.selected = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int front = 0, back = 0;
    BnBNode root;
    root.level = 0;
    root.risk = 0.0;
    root.hours = 0.0;
    root.bound = fractionalUpperBound(items, validCount, 0, 0.0, 0.0, capacityHours);
    root.mask = (int*)calloc((size_t)validCount, sizeof(int));

    if (mode == BNB_LEAST_COST)
    {
        heapPushBnB(container, &back, root, metrics);
    }
    else
    {
        container[back++] = root;
    }

    while ((mode == BNB_FIFO && front < back) || (mode != BNB_FIFO && back > 0))
    {
        BnBNode cur;
        if (mode == BNB_FIFO)
        {
            cur = container[front++];
        }
        else if (mode == BNB_LIFO)
        {
            cur = container[--back];
        }
        else
        {
            cur = heapPopBnB(container, &back, metrics);
        }

        res.nodesExplored++;
        countComparison(metrics);
        if (cur.bound <= res.maxRisk || cur.level >= validCount)
        {
            free(cur.mask);
            continue;
        }

        /* Ensure container capacity for up to 2 children */
        if (back + 2 >= cap)
        {
            int newCap = cap * 2;
            BnBNode* resized = (BnBNode*)realloc(container, (size_t)newCap * sizeof(BnBNode));
            if (resized)
            {
                container = resized;
                cap = newCap;
            }
        }

        int lvl = cur.level;

        /* Child 1: Include item[lvl] */
        if (cur.hours + items[lvl].item.hoursRequired <= capacityHours && back + 1 < cap)
        {
            BnBNode inc;
            inc.level = lvl + 1;
            inc.risk = cur.risk + items[lvl].item.riskScore;
            inc.hours = cur.hours + items[lvl].item.hoursRequired;
            inc.bound = fractionalUpperBound(items, validCount, inc.level, inc.risk, inc.hours, capacityHours);
            inc.mask = (int*)malloc((size_t)validCount * sizeof(int));
            if (inc.mask)
            {
                memcpy(inc.mask, cur.mask, (size_t)validCount * sizeof(int));
                inc.mask[lvl] = 1;
                if (inc.risk > res.maxRisk)
                {
                    res.maxRisk = inc.risk;
                    for (int i = 0; i < validCount; i++)
                    {
                        res.selected[items[i].originalIndex] = inc.mask[i];
                    }
                }
                if (inc.bound > res.maxRisk && inc.level < validCount)
                {
                    if (mode == BNB_LEAST_COST) heapPushBnB(container, &back, inc, metrics);
                    else container[back++] = inc;
                }
                else
                {
                    free(inc.mask);
                }
            }
        }

        /* Child 2: Exclude item[lvl] */
        if (back + 1 < cap)
        {
            BnBNode exc;
            exc.level = lvl + 1;
            exc.risk = cur.risk;
            exc.hours = cur.hours;
            exc.bound = fractionalUpperBound(items, validCount, exc.level, exc.risk, exc.hours, capacityHours);
            if (exc.bound > res.maxRisk && exc.level < validCount)
            {
                exc.mask = (int*)malloc((size_t)validCount * sizeof(int));
                if (exc.mask)
                {
                    memcpy(exc.mask, cur.mask, (size_t)validCount * sizeof(int));
                    exc.mask[lvl] = 0;
                    if (mode == BNB_LEAST_COST) heapPushBnB(container, &back, exc, metrics);
                    else container[back++] = exc;
                }
            }
        }

        free(cur.mask);
    }

    if (mode == BNB_FIFO)
    {
        for (int i = front; i < back; i++) free(container[i].mask);
    }
    else
    {
        for (int i = 0; i < back; i++) free(container[i].mask);
    }

    trackBytes(metrics, (size_t)cap * sizeof(BnBNode));
    free(container);
    free(items);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

void freeExactKnapsackResult(ExactKnapsackResult* res)
{
    if (!res) return;
    free(res->selected);
    res->selected = NULL;
    res->count = 0;
}
