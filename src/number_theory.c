#include "../include/number_theory.h"

#include <stdlib.h>

long long extendedGCD(
    long long a,
    long long b,
    long long* x,
    long long* y
)
{
    if (b == 0)
    {
        if (x) *x = 1;
        if (y) *y = 0;
        return a >= 0 ? a : -a;
    }

    long long x1 = 0, y1 = 0;
    long long g = extendedGCD(b, a % b, &x1, &y1);
    if (x) *x = y1;
    if (y) *y = x1 - (a / b) * y1;
    return g;
}

long long modInverse(
    long long a,
    long long m,
    int* exists
)
{
    if (m <= 0)
    {
        if (exists) *exists = 0;
        return 0;
    }

    long long x = 0, y = 0;
    long long g = extendedGCD(a, m, &x, &y);
    if (g != 1)
    {
        if (exists) *exists = 0;
        return 0;
    }

    if (exists) *exists = 1;
    long long res = x % m;
    return (res < 0) ? (res + m) : res;
}

static long long normalizeMod(long long val, long long mod)
{
    long long r = val % mod;
    return (r < 0) ? (r + mod) : r;
}

CrtResult chineseRemainderTheorem(
    const long long* remainders,
    const long long* moduli,
    int count,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    CrtResult res = {0, 0, 0};
    if (!remainders || !moduli || count <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    if (moduli[0] <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    long long curRem = normalizeMod(remainders[0], moduli[0]);
    long long curMod = moduli[0];

    for (int i = 1; i < count; i++)
    {
        if (moduli[i] <= 0)
        {
            if (metrics) metrics->elapsedMicros = timerStop(&timer);
            return res;
        }

        long long r2 = normalizeMod(remainders[i], moduli[i]);
        long long m2 = moduli[i];

        long long x = 0, y = 0;
        long long g = extendedGCD(curMod, m2, &x, &y);
        countComparison(metrics);

        if ((r2 - curRem) % g != 0)
        {
            if (metrics) metrics->elapsedMicros = timerStop(&timer);
            return res;
        }

        long long lcmMod = (curMod / g) * m2;
        long long m2DivG = m2 / g;
        long long step = normalizeMod(((r2 - curRem) / g) * x, m2DivG);

        curRem = normalizeMod(curRem + curMod * step, lcmMod);
        curMod = lcmMod;
        countSwap(metrics);
    }

    res.value = curRem;
    res.combinedModulus = curMod;
    res.valid = 1;
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

CrtResult reconstructDistributedTxTotal(
    const Transaction* transactions,
    int txCount,
    const long long* moduli,
    int moduliCount,
    Metrics* metrics
)
{
    CrtResult invalid = {0, 0, 0};
    if (!transactions || txCount <= 0 || !moduli || moduliCount <= 0)
    {
        return invalid;
    }

    long long* remainders = (long long*)calloc((size_t)moduliCount, sizeof(long long));
    if (!remainders) return invalid;

    for (int k = 0; k < moduliCount; k++)
    {
        if (moduli[k] <= 0)
        {
            free(remainders);
            return invalid;
        }
        long long sumMod = 0;
        for (int i = 0; i < txCount; i++)
        {
            long long amt = (long long)(transactions[i].amount + 0.5);
            sumMod = normalizeMod(sumMod + amt, moduli[k]);
        }
        remainders[k] = sumMod;
    }

    CrtResult res = chineseRemainderTheorem(remainders, moduli, moduliCount, metrics);
    free(remainders);
    return res;
}
