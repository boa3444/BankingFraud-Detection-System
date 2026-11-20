#ifndef SEARCHING_H
#define SEARCHING_H

#include "transaction.h"

/*
    Searches for a transaction ID using iterative binary search.
    The array must already be sorted by transactionId.
*/
int iterativeBinarySearchById(
    const Transaction* transactions,
    int count,
    int targetId
);

/*
    Searches for a transaction ID using recursive binary search.
    The array must already be sorted by transactionId.
*/
int recursiveBinarySearchById(
    const Transaction* transactions,
    int count,
    int targetId
);

/*
    Searches for a sender account ID using iterative binary search.
    The array must already be sorted alphabetically by senderAccount.
*/
int iterativeBinarySearchByAccount(
    const Transaction* transactions,
    int count,
    const char* targetAccount
);

/*
    Recursive helper for account search.
*/
int recursiveAccountSearch(
    const Transaction* transactions,
    int count,
    const char* targetAccount,
    int leftIndex,
    int rightIndex
);

/*
    Public recursive account search.
*/
int recursiveBinarySearchByAccount(
    const Transaction* transactions,
    int count,
    const char* targetAccount
);

#endif
