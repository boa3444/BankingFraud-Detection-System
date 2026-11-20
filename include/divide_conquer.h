#ifndef DIVIDE_CONQUER_H
#define DIVIDE_CONQUER_H

#include "analysis.h"
#include "transaction.h"

#define STRASSEN_CUTOFF 16
#define SUSPICIOUS_AMOUNT_THRESHOLD 20000.0

/*
    Represents a contiguous time range and its total value.
*/
typedef struct
{
    int startIndex;
    int endIndex;
    double sum;
    int valid;
} SubarrayResult;

/*
    Stores the result of celebrity identification.
*/
typedef struct
{
    int celebrity;
    long long lookups;
} CelebrityResult;

/*
    Flat row-major matrix of long long values.
    Element (r, c) is at data[r * cols + c].
*/
typedef struct
{
    long long* data;
    int rows;
    int cols;
} Matrix;

/*
    Stores the comparison of classical and Strassen multiplication.
*/
typedef struct
{
    int equal;
    long long classicalMicros;
    long long strassenMicros;
    long long classicalMultiplications;
    long long strassenMultiplications;
} MatrixComparison;

/* Maximum Subarray & Suspicious Buckets */
SubarrayResult maxSubarrayDivideConquer(
    const double* values,
    int count,
    Metrics* metrics
);

SubarrayResult maxSubarrayKadane(
    const double* values,
    int count,
    Metrics* metrics
);

double* buildSuspiciousBuckets(
    const Transaction* transactions,
    int count,
    int bucketSize,
    int* outBucketCount,
    Metrics* metrics
);

/* Modular Exponentiation & Hashing */
unsigned long long modPowIterative(
    unsigned long long base,
    unsigned long long exp,
    unsigned long long mod,
    Metrics* metrics
);

unsigned long long modPowRecursive(
    unsigned long long base,
    unsigned long long exp,
    unsigned long long mod,
    Metrics* metrics
);

unsigned long long naiveModPow(
    unsigned long long base,
    unsigned long long exp,
    unsigned long long mod,
    Metrics* metrics
);

unsigned long long polynomialHashAccount(
    const char* accountId,
    unsigned long long base,
    unsigned long long mod
);

/* Celebrity Problem (flat n x n matrix: knows[i * n + j]) */
CelebrityResult celebrityElimination(
    const int* knows,
    int n,
    Metrics* metrics
);

CelebrityResult celebrityBruteForce(
    const int* knows,
    int n,
    Metrics* metrics
);

/* Matrix Helpers & Multiplication (Classical vs Strassen) */
Matrix createMatrix(int rows, int cols);
void freeMatrix(Matrix* m);
int matricesEqual(const Matrix* a, const Matrix* b);

Matrix multiplyClassical(
    const Matrix* first,
    const Matrix* second,
    Metrics* metrics
);

Matrix multiplyStrassen(
    const Matrix* first,
    const Matrix* second,
    Metrics* metrics
);

MatrixComparison compareMatrixMultiplication(
    const Matrix* first,
    const Matrix* second
);

Matrix twoHopConnections(
    const Matrix* transactionCounts,
    int useStrassen,
    Metrics* metrics
);

#endif
