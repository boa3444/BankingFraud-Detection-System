#ifndef SORTING_H
#define SORTING_H

#include "transaction.h"

/*
    Stores operation counts for an algorithm.
*/
typedef struct
{
    long long comparisons;
    long long swaps;
    long long recursiveCalls;
} SortingStatistics;

/*
    Sorts transactions by amount using Merge Sort.
*/
void mergeSortByAmount(
    Transaction* transactions,
    int count,
    SortingStatistics* statistics
);

/*
    Sorts transactions by transaction ID using Merge Sort.
*/
void mergeSortByTransactionId(
    Transaction* transactions,
    int count,
    SortingStatistics* statistics
);

/*
    Sorts transactions by amount using Quick Sort.
*/
void quickSortByAmount(
    Transaction* transactions,
    int count,
    SortingStatistics* statistics
);

/*
    Sorts transactions by transaction ID using Quick Sort.
*/
void quickSortByTransactionId(
    Transaction* transactions,
    int count,
    SortingStatistics* statistics
);

/*
    Displays sorting statistics.
*/
void displaySortingStatistics(
    const char* algorithmName,
    SortingStatistics statistics
);

#endif
