#include "../include/divide_conquer.h"

#include <stdlib.h>
#include <string.h>

/* ============================================================
   MAXIMUM SUBARRAY (DIVIDE & CONQUER + KADANE)
   ============================================================ */

static int isBetterSubarray(const SubarrayResult* first, const SubarrayResult* second)
{
    if (!second->valid)
    {
        return first->valid;
    }
    if (!first->valid)
    {
        return 0;
    }
    if (first->sum != second->sum)
    {
        return first->sum > second->sum;
    }
    if (first->startIndex != second->startIndex)
    {
        return first->startIndex < second->startIndex;
    }
    return first->endIndex < second->endIndex;
}

static SubarrayResult crossingSubarray(
    const double* values,
    int left,
    int mid,
    int right,
    Metrics* metrics
)
{
    double leftSum = 0.0;
    double bestLeftSum = values[mid];
    int bestLeftIdx = mid;

    for (int i = mid; i >= left; i--)
    {
        leftSum += values[i];
        countComparison(metrics);
        if (i == mid || leftSum > bestLeftSum ||
            (leftSum == bestLeftSum && i < bestLeftIdx))
        {
            bestLeftSum = leftSum;
            bestLeftIdx = i;
        }
    }

    double rightSum = 0.0;
    double bestRightSum = values[mid + 1];
    int bestRightIdx = mid + 1;

    for (int j = mid + 1; j <= right; j++)
    {
        rightSum += values[j];
        countComparison(metrics);
        if (j == mid + 1 || rightSum > bestRightSum)
        {
            bestRightSum = rightSum;
            bestRightIdx = j;
        }
    }

    SubarrayResult res;
    res.startIndex = bestLeftIdx;
    res.endIndex = bestRightIdx;
    res.sum = bestLeftSum + bestRightSum;
    res.valid = 1;
    return res;
}

static SubarrayResult maxSubarrayHelper(
    const double* values,
    int left,
    int right,
    Metrics* metrics
)
{
    countCall(metrics);

    if (left == right)
    {
        SubarrayResult single = {left, right, values[left], 1};
        return single;
    }

    int mid = left + (right - left) / 2;
    SubarrayResult leftBest = maxSubarrayHelper(values, left, mid, metrics);
    SubarrayResult rightBest = maxSubarrayHelper(values, mid + 1, right, metrics);
    SubarrayResult crossBest = crossingSubarray(values, left, mid, right, metrics);

    SubarrayResult best = leftBest;
    countComparison(metrics);
    if (isBetterSubarray(&rightBest, &best))
    {
        best = rightBest;
    }
    countComparison(metrics);
    if (isBetterSubarray(&crossBest, &best))
    {
        best = crossBest;
    }
    return best;
}

SubarrayResult maxSubarrayDivideConquer(
    const double* values,
    int count,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    SubarrayResult invalid = {0, -1, 0.0, 0};
    if (values == NULL || count <= 0)
    {
        if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
        return invalid;
    }

    SubarrayResult res = maxSubarrayHelper(values, 0, count - 1, metrics);
    trackBytes(metrics, (size_t)sizeof(SubarrayResult) * 32);
    if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

SubarrayResult maxSubarrayKadane(
    const double* values,
    int count,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    SubarrayResult invalid = {0, -1, 0.0, 0};
    if (values == NULL || count <= 0)
    {
        if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
        return invalid;
    }

    SubarrayResult best = {0, 0, values[0], 1};
    double currentSum = values[0];
    int currentStart = 0;

    for (int i = 1; i < count; i++)
    {
        countComparison(metrics);
        if (currentSum + values[i] >= values[i])
        {
            currentSum += values[i];
        }
        else
        {
            currentSum = values[i];
            currentStart = i;
        }

        SubarrayResult candidate = {currentStart, i, currentSum, 1};
        countComparison(metrics);
        if (isBetterSubarray(&candidate, &best))
        {
            best = candidate;
        }
    }

    if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
    return best;
}

double* buildSuspiciousBuckets(
    const Transaction* transactions,
    int count,
    int bucketSize,
    int* outBucketCount,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    if (outBucketCount != NULL) *outBucketCount = 0;
    if (transactions == NULL || count <= 0 || bucketSize <= 0)
    {
        if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
        return NULL;
    }

    long long minTs = transactions[0].timestamp;
    long long maxTs = transactions[0].timestamp;
    for (int i = 1; i < count; i++)
    {
        if (transactions[i].timestamp < minTs) minTs = transactions[i].timestamp;
        if (transactions[i].timestamp > maxTs) maxTs = transactions[i].timestamp;
    }

    int bucketCount = (int)((maxTs - minTs) / bucketSize) + 1;
    double* buckets = (double*)calloc((size_t)bucketCount, sizeof(double));
    if (buckets == NULL)
    {
        if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
        return NULL;
    }

    for (int i = 0; i < count; i++)
    {
        int b = (int)((transactions[i].timestamp - minTs) / bucketSize);
        if (transactions[i].isFraud != 0 || transactions[i].amount > SUSPICIOUS_AMOUNT_THRESHOLD)
        {
            buckets[b] += transactions[i].amount;
        }
        else
        {
            buckets[b] -= transactions[i].amount;
        }
        countSwap(metrics);
    }

    if (outBucketCount != NULL) *outBucketCount = bucketCount;
    trackBytes(metrics, (size_t)bucketCount * sizeof(double));
    if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
    return buckets;
}

/* ============================================================
   MODULAR EXPONENTIATION & HASHING
   ============================================================ */

unsigned long long modPowIterative(
    unsigned long long base,
    unsigned long long exp,
    unsigned long long mod,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    if (mod == 0) return 0;

    unsigned long long result = 1 % mod;
    unsigned long long b = base % mod;
    unsigned long long e = exp;

    while (e > 0)
    {
        if (e & 1ULL)
        {
            result = (result * b) % mod;
            countSwap(metrics);
        }
        b = (b * b) % mod;
        e >>= 1;
        countComparison(metrics);
    }

    return result;
}

static unsigned long long modPowRecHelper(
    unsigned long long base,
    unsigned long long exp,
    unsigned long long mod,
    Metrics* metrics
)
{
    countCall(metrics);
    if (exp == 0) return 1 % mod;

    unsigned long long half = modPowRecHelper(base, exp / 2, mod, metrics);
    unsigned long long sq = (half * half) % mod;
    countSwap(metrics);

    if (exp & 1ULL)
    {
        return (sq * (base % mod)) % mod;
    }
    return sq;
}

unsigned long long modPowRecursive(
    unsigned long long base,
    unsigned long long exp,
    unsigned long long mod,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    if (mod == 0) return 0;
    return modPowRecHelper(base, exp, mod, metrics);
}

unsigned long long naiveModPow(
    unsigned long long base,
    unsigned long long exp,
    unsigned long long mod,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    if (mod == 0) return 0;

    unsigned long long result = 1 % mod;
    unsigned long long b = base % mod;
    for (unsigned long long i = 0; i < exp; i++)
    {
        result = (result * b) % mod;
        countSwap(metrics);
    }
    return result;
}

unsigned long long polynomialHashAccount(
    const char* accountId,
    unsigned long long base,
    unsigned long long mod
)
{
    if (accountId == NULL || mod == 0) return 0;

    unsigned long long hash = 0;
    size_t len = strlen(accountId);
    for (size_t i = 0; i < len; i++)
    {
        unsigned long long p = modPowIterative(base, (unsigned long long)i, mod, NULL);
        hash = (hash + ((unsigned char)accountId[i] * p) % mod) % mod;
    }
    return hash;
}

/* ============================================================
   CELEBRITY PROBLEM
   ============================================================ */

CelebrityResult celebrityElimination(
    const int* knows,
    int n,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    CelebrityResult res = {-1, 0};
    if (knows == NULL || n <= 0)
    {
        if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int candidate = 0;
    for (int i = 1; i < n; i++)
    {
        res.lookups++;
        countComparison(metrics);
        if (knows[candidate * n + i] == 1)
        {
            candidate = i;
        }
    }

    for (int i = 0; i < n; i++)
    {
        if (i == candidate) continue;
        res.lookups += 2;
        countComparison(metrics);
        if (knows[candidate * n + i] == 1 || knows[i * n + candidate] == 0)
        {
            if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
            return res;
        }
    }

    res.celebrity = candidate;
    if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

CelebrityResult celebrityBruteForce(
    const int* knows,
    int n,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    CelebrityResult res = {-1, 0};
    if (knows == NULL || n <= 0)
    {
        if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int c = 0; c < n; c++)
    {
        int isCeleb = 1;
        for (int i = 0; i < n; i++)
        {
            if (i == c) continue;
            res.lookups += 2;
            countComparison(metrics);
            if (knows[c * n + i] == 1 || knows[i * n + c] == 0)
            {
                isCeleb = 0;
                break;
            }
        }
        if (isCeleb)
        {
            res.celebrity = c;
            break;
        }
    }

    if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
    return res;
}
