#include "../include/sorting.h"

#include <stdio.h>
#include <stdlib.h>

static void resetStats(SortingStatistics* statistics)
{
    if (statistics != NULL)
    {
        statistics->comparisons = 0;
        statistics->swaps = 0;
        statistics->recursiveCalls = 0;
    }
}

static void swapTransactions(
    Transaction* a,
    Transaction* b,
    SortingStatistics* statistics
)
{
    Transaction temp = *a;
    *a = *b;
    *b = temp;
    if (statistics != NULL)
    {
        statistics->swaps++;
    }
}

static int amountComesBefore(
    const Transaction* first,
    const Transaction* second,
    SortingStatistics* statistics
)
{
    if (statistics != NULL)
    {
        statistics->comparisons++;
    }
    return first->amount <= second->amount;
}

static int idComesBefore(
    const Transaction* first,
    const Transaction* second,
    SortingStatistics* statistics
)
{
    if (statistics != NULL)
    {
        statistics->comparisons++;
    }
    return first->transactionId <= second->transactionId;
}

/* ============================================================
   MERGE SORT
   ============================================================ */

static void mergeRange(
    Transaction* transactions,
    Transaction* temp,
    int left,
    int mid,
    int right,
    int byAmount,
    SortingStatistics* statistics
)
{
    int i = left;
    int j = mid + 1;
    int k = 0;

    while (i <= mid && j <= right)
    {
        int takeLeft = byAmount
            ? amountComesBefore(&transactions[i], &transactions[j], statistics)
            : idComesBefore(&transactions[i], &transactions[j], statistics);

        if (takeLeft)
        {
            temp[k++] = transactions[i++];
        }
        else
        {
            temp[k++] = transactions[j++];
        }
        if (statistics != NULL)
        {
            statistics->swaps++;
        }
    }

    while (i <= mid)
    {
        temp[k++] = transactions[i++];
        if (statistics != NULL)
        {
            statistics->swaps++;
        }
    }

    while (j <= right)
    {
        temp[k++] = transactions[j++];
        if (statistics != NULL)
        {
            statistics->swaps++;
        }
    }

    for (int idx = 0; idx < k; idx++)
    {
        transactions[left + idx] = temp[idx];
        if (statistics != NULL)
        {
            statistics->swaps++;
        }
    }
}

static void mergeSortHelper(
    Transaction* transactions,
    Transaction* temp,
    int left,
    int right,
    int byAmount,
    SortingStatistics* statistics
)
{
    if (statistics != NULL)
    {
        statistics->recursiveCalls++;
    }

    if (left >= right)
    {
        return;
    }

    int mid = left + (right - left) / 2;
    mergeSortHelper(transactions, temp, left, mid, byAmount, statistics);
    mergeSortHelper(transactions, temp, mid + 1, right, byAmount, statistics);
    mergeRange(transactions, temp, left, mid, right, byAmount, statistics);
}

void mergeSortByAmount(
    Transaction* transactions,
    int count,
    SortingStatistics* statistics
)
{
    resetStats(statistics);
    if (transactions == NULL || count <= 1)
    {
        return;
    }

    Transaction* temp = (Transaction*)malloc((size_t)count * sizeof(Transaction));
    if (temp == NULL)
    {
        fprintf(stderr, "Error: Out of memory in mergeSortByAmount.\n");
        return;
    }

    mergeSortHelper(transactions, temp, 0, count - 1, 1, statistics);
    free(temp);
}

void mergeSortByTransactionId(
    Transaction* transactions,
    int count,
    SortingStatistics* statistics
)
{
    resetStats(statistics);
    if (transactions == NULL || count <= 1)
    {
        return;
    }

    Transaction* temp = (Transaction*)malloc((size_t)count * sizeof(Transaction));
    if (temp == NULL)
    {
        fprintf(stderr, "Error: Out of memory in mergeSortByTransactionId.\n");
        return;
    }

    mergeSortHelper(transactions, temp, 0, count - 1, 0, statistics);
    free(temp);
}

/* ============================================================
   QUICK SORT
   ============================================================ */

static int partitionRange(
    Transaction* transactions,
    int low,
    int high,
    int byAmount,
    SortingStatistics* statistics
)
{
    Transaction pivot = transactions[high];
    int i = low - 1;

    for (int j = low; j < high; j++)
    {
        int comesBefore = byAmount
            ? amountComesBefore(&transactions[j], &pivot, statistics)
            : idComesBefore(&transactions[j], &pivot, statistics);

        if (comesBefore)
        {
            i++;
            swapTransactions(&transactions[i], &transactions[j], statistics);
        }
    }

    swapTransactions(&transactions[i + 1], &transactions[high], statistics);
    return i + 1;
}

static void quickSortHelper(
    Transaction* transactions,
    int low,
    int high,
    int byAmount,
    SortingStatistics* statistics
)
{
    if (statistics != NULL)
    {
        statistics->recursiveCalls++;
    }

    if (low >= high)
    {
        return;
    }

    int pivotIndex = partitionRange(transactions, low, high, byAmount, statistics);
    quickSortHelper(transactions, low, pivotIndex - 1, byAmount, statistics);
    quickSortHelper(transactions, pivotIndex + 1, high, byAmount, statistics);
}

void quickSortByAmount(
    Transaction* transactions,
    int count,
    SortingStatistics* statistics
)
{
    resetStats(statistics);
    if (transactions == NULL || count <= 1)
    {
        return;
    }

    quickSortHelper(transactions, 0, count - 1, 1, statistics);
}

void quickSortByTransactionId(
    Transaction* transactions,
    int count,
    SortingStatistics* statistics
)
{
    resetStats(statistics);
    if (transactions == NULL || count <= 1)
    {
        return;
    }

    quickSortHelper(transactions, 0, count - 1, 0, statistics);
}

void displaySortingStatistics(
    const char* algorithmName,
    SortingStatistics statistics
)
{
    printf("\nAlgorithm: %s\n", algorithmName ? algorithmName : "Unknown");
    printf("Comparisons: %lld\n", statistics.comparisons);
    printf("Swaps/Moves: %lld\n", statistics.swaps);
    printf("Recursive Calls: %lld\n", statistics.recursiveCalls);
}
