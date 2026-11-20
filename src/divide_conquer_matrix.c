#include "../include/divide_conquer.h"

#include <stdlib.h>
#include <string.h>

Matrix createMatrix(int rows, int cols)
{
    Matrix m = {NULL, rows, cols};
    if (rows > 0 && cols > 0)
    {
        m.data = (long long*)calloc((size_t)rows * (size_t)cols, sizeof(long long));
    }
    return m;
}

void freeMatrix(Matrix* m)
{
    if (m == NULL) return;
    free(m->data);
    m->data = NULL;
    m->rows = 0;
    m->cols = 0;
}

int matricesEqual(const Matrix* a, const Matrix* b)
{
    if (a == NULL || b == NULL) return 0;
    if (a->rows != b->rows || a->cols != b->cols) return 0;
    size_t total = (size_t)a->rows * (size_t)a->cols;
    for (size_t i = 0; i < total; i++)
    {
        if (a->data[i] != b->data[i]) return 0;
    }
    return 1;
}

static Matrix classicalCore(
    const Matrix* a,
    const Matrix* b,
    Metrics* metrics
)
{
    Matrix c = createMatrix(a->rows, b->cols);
    if (c.data == NULL) return c;

    for (int i = 0; i < a->rows; i++)
    {
        for (int k = 0; k < a->cols; k++)
        {
            long long aik = a->data[i * a->cols + k];
            for (int j = 0; j < b->cols; j++)
            {
                c.data[i * c.cols + j] += aik * b->data[k * b->cols + j];
                countSwap(metrics);
            }
        }
    }

    return c;
}

Matrix multiplyClassical(
    const Matrix* first,
    const Matrix* second,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    if (first == NULL || second == NULL || first->cols != second->rows ||
        first->rows <= 0 || second->cols <= 0)
    {
        if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
        return createMatrix(0, 0);
    }

    Matrix res = classicalCore(first, second, metrics);
    trackBytes(metrics, (size_t)res.rows * (size_t)res.cols * sizeof(long long));
    if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

static Matrix addSubMatrix(const Matrix* a, const Matrix* b, int sign)
{
    int n = a->rows;
    Matrix c = createMatrix(n, n);
    for (int i = 0; i < n * n; i++)
    {
        c.data[i] = a->data[i] + sign * b->data[i];
    }
    return c;
}

static Matrix strassenRecursive(
    const Matrix* a,
    const Matrix* b,
    Metrics* metrics
)
{
    countCall(metrics);
    int n = a->rows;
    if (n <= STRASSEN_CUTOFF)
    {
        return classicalCore(a, b, metrics);
    }

    int half = n / 2;
    Matrix a11 = createMatrix(half, half);
    Matrix a12 = createMatrix(half, half);
    Matrix a21 = createMatrix(half, half);
    Matrix a22 = createMatrix(half, half);
    Matrix b11 = createMatrix(half, half);
    Matrix b12 = createMatrix(half, half);
    Matrix b21 = createMatrix(half, half);
    Matrix b22 = createMatrix(half, half);

    for (int i = 0; i < half; i++)
    {
        for (int j = 0; j < half; j++)
        {
            a11.data[i * half + j] = a->data[i * n + j];
            a12.data[i * half + j] = a->data[i * n + (j + half)];
            a21.data[i * half + j] = a->data[(i + half) * n + j];
            a22.data[i * half + j] = a->data[(i + half) * n + (j + half)];

            b11.data[i * half + j] = b->data[i * n + j];
            b12.data[i * half + j] = b->data[i * n + (j + half)];
            b21.data[i * half + j] = b->data[(i + half) * n + j];
            b22.data[i * half + j] = b->data[(i + half) * n + (j + half)];
        }
    }

    Matrix t1 = addSubMatrix(&a11, &a22, 1);
    Matrix t2 = addSubMatrix(&b11, &b22, 1);
    Matrix m1 = strassenRecursive(&t1, &t2, metrics);
    freeMatrix(&t1); freeMatrix(&t2);

    t1 = addSubMatrix(&a21, &a22, 1);
    Matrix m2 = strassenRecursive(&t1, &b11, metrics);
    freeMatrix(&t1);

    t2 = addSubMatrix(&b12, &b22, -1);
    Matrix m3 = strassenRecursive(&a11, &t2, metrics);
    freeMatrix(&t2);

    t2 = addSubMatrix(&b21, &b11, -1);
    Matrix m4 = strassenRecursive(&a22, &t2, metrics);
    freeMatrix(&t2);

    t1 = addSubMatrix(&a11, &a12, 1);
    Matrix m5 = strassenRecursive(&t1, &b22, metrics);
    freeMatrix(&t1);

    t1 = addSubMatrix(&a21, &a11, -1);
    t2 = addSubMatrix(&b11, &b12, 1);
    Matrix m6 = strassenRecursive(&t1, &t2, metrics);
    freeMatrix(&t1); freeMatrix(&t2);

    t1 = addSubMatrix(&a12, &a22, -1);
    t2 = addSubMatrix(&b21, &b22, 1);
    Matrix m7 = strassenRecursive(&t1, &t2, metrics);
    freeMatrix(&t1); freeMatrix(&t2);

    freeMatrix(&a11); freeMatrix(&a12); freeMatrix(&a21); freeMatrix(&a22);
    freeMatrix(&b11); freeMatrix(&b12); freeMatrix(&b21); freeMatrix(&b22);

    Matrix c = createMatrix(n, n);
    for (int i = 0; i < half; i++)
    {
        for (int j = 0; j < half; j++)
        {
            int idx = i * half + j;
            c.data[i * n + j] = m1.data[idx] + m4.data[idx] - m5.data[idx] + m7.data[idx];
            c.data[i * n + (j + half)] = m3.data[idx] + m5.data[idx];
            c.data[(i + half) * n + j] = m2.data[idx] + m4.data[idx];
            c.data[(i + half) * n + (j + half)] = m1.data[idx] - m2.data[idx] + m3.data[idx] + m6.data[idx];
        }
    }

    freeMatrix(&m1); freeMatrix(&m2); freeMatrix(&m3); freeMatrix(&m4);
    freeMatrix(&m5); freeMatrix(&m6); freeMatrix(&m7);
    return c;
}

Matrix multiplyStrassen(
    const Matrix* first,
    const Matrix* second,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    if (first == NULL || second == NULL || first->rows != first->cols ||
        second->rows != second->cols || first->rows != second->rows || first->rows <= 0)
    {
        if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
        return createMatrix(0, 0);
    }

    int n = first->rows;
    int padded = 1;
    while (padded < n)
    {
        padded *= 2;
    }

    Matrix pa = createMatrix(padded, padded);
    Matrix pb = createMatrix(padded, padded);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            pa.data[i * padded + j] = first->data[i * n + j];
            pb.data[i * padded + j] = second->data[i * n + j];
        }
    }

    Matrix pc = strassenRecursive(&pa, &pb, metrics);
    freeMatrix(&pa);
    freeMatrix(&pb);

    Matrix result = createMatrix(n, n);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            result.data[i * n + j] = pc.data[i * padded + j];
        }
    }
    freeMatrix(&pc);

    trackBytes(metrics, (size_t)padded * (size_t)padded * sizeof(long long));
    if (metrics != NULL) metrics->elapsedMicros = timerStop(&timer);
    return result;
}

MatrixComparison compareMatrixMultiplication(
    const Matrix* first,
    const Matrix* second
)
{
    Metrics mClassic, mStrassen;
    Matrix c1 = multiplyClassical(first, second, &mClassic);
    Matrix c2 = multiplyStrassen(first, second, &mStrassen);

    MatrixComparison cmp;
    cmp.equal = matricesEqual(&c1, &c2);
    cmp.classicalMicros = mClassic.elapsedMicros;
    cmp.strassenMicros = mStrassen.elapsedMicros;
    cmp.classicalMultiplications = mClassic.swapsOrAssignments;
    cmp.strassenMultiplications = mStrassen.swapsOrAssignments;

    freeMatrix(&c1);
    freeMatrix(&c2);
    return cmp;
}

Matrix twoHopConnections(
    const Matrix* transactionCounts,
    int useStrassen,
    Metrics* metrics
)
{
    if (useStrassen)
    {
        return multiplyStrassen(transactionCounts, transactionCounts, metrics);
    }
    return multiplyClassical(transactionCounts, transactionCounts, metrics);
}
