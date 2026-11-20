#ifndef NUMBER_THEORY_H
#define NUMBER_THEORY_H

#include "analysis.h"
#include "transaction.h"

typedef struct
{
    long long value;
    long long combinedModulus;
    int valid;
} CrtResult;

/*
    Extended Euclidean Algorithm:
    Returns gcd(a, b) and finds x, y such that a*x + b*y = gcd(a, b).
*/
long long extendedGCD(
    long long a,
    long long b,
    long long* x,
    long long* y
);

/*
    Computes modular multiplicative inverse of a modulo m.
    Sets *exists = 1 if inverse exists, 0 otherwise.
*/
long long modInverse(
    long long a,
    long long m,
    int* exists
);

/*
    Solves x === remainders[i] (mod moduli[i]) for i = 0..count-1.
    Supports both pairwise-coprime and general moduli; sets valid = 0 if inconsistent.
*/
CrtResult chineseRemainderTheorem(
    const long long* remainders,
    const long long* moduli,
    int count,
    Metrics* metrics
);

/*
    Project 07 utility:
    Computes modular transaction amount checksums across distributed shards
    and reconstructs the global total modulo prod(moduli) via CRT.
*/
CrtResult reconstructDistributedTxTotal(
    const Transaction* transactions,
    int txCount,
    const long long* moduli,
    int moduliCount,
    Metrics* metrics
);

#endif
