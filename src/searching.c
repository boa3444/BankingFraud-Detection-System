#include "../include/searching.h"

#include <stddef.h>
#include <string.h>

int iterativeBinarySearchById(
    const Transaction* transactions,
    int count,
    int targetId
)
{
    if (transactions == NULL || count <= 0)
    {
        return -1;
    }

    int leftIndex = 0;
    int rightIndex = count - 1;

    while (leftIndex <= rightIndex)
    {
        int middleIndex = leftIndex + (rightIndex - leftIndex) / 2;
        int middleId = transactions[middleIndex].transactionId;

        if (middleId == targetId)
        {
            return middleIndex;
        }

        if (middleId < targetId)
        {
            leftIndex = middleIndex + 1;
        }
        else
        {
            rightIndex = middleIndex - 1;
        }
    }

    return -1;
}

static int recursiveBinarySearchIdHelper(
    const Transaction* transactions,
    int targetId,
    int leftIndex,
    int rightIndex
)
{
    if (leftIndex > rightIndex)
    {
        return -1;
    }

    int middleIndex = leftIndex + (rightIndex - leftIndex) / 2;
    int middleId = transactions[middleIndex].transactionId;

    if (middleId == targetId)
    {
        return middleIndex;
    }

    if (middleId < targetId)
    {
        return recursiveBinarySearchIdHelper(
            transactions,
            targetId,
            middleIndex + 1,
            rightIndex
        );
    }

    return recursiveBinarySearchIdHelper(
        transactions,
        targetId,
        leftIndex,
        middleIndex - 1
    );
}

int recursiveBinarySearchById(
    const Transaction* transactions,
    int count,
    int targetId
)
{
    if (transactions == NULL || count <= 0)
    {
        return -1;
    }

    return recursiveBinarySearchIdHelper(transactions, targetId, 0, count - 1);
}

int iterativeBinarySearchByAccount(
    const Transaction* transactions,
    int count,
    const char* targetAccount
)
{
    if (transactions == NULL || count <= 0 || targetAccount == NULL)
    {
        return -1;
    }

    int leftIndex = 0;
    int rightIndex = count - 1;

    while (leftIndex <= rightIndex)
    {
        int middleIndex = leftIndex + (rightIndex - leftIndex) / 2;
        int cmp = strcmp(transactions[middleIndex].senderAccount, targetAccount);

        if (cmp == 0)
        {
            return middleIndex;
        }

        if (cmp < 0)
        {
            leftIndex = middleIndex + 1;
        }
        else
        {
            rightIndex = middleIndex - 1;
        }
    }

    return -1;
}

int recursiveAccountSearch(
    const Transaction* transactions,
    int count,
    const char* targetAccount,
    int leftIndex,
    int rightIndex
)
{
    if (transactions == NULL || count <= 0 || targetAccount == NULL)
    {
        return -1;
    }

    if (leftIndex > rightIndex || rightIndex >= count)
    {
        return -1;
    }

    int middleIndex = leftIndex + (rightIndex - leftIndex) / 2;
    int cmp = strcmp(transactions[middleIndex].senderAccount, targetAccount);

    if (cmp == 0)
    {
        return middleIndex;
    }

    if (cmp < 0)
    {
        return recursiveAccountSearch(
            transactions,
            count,
            targetAccount,
            middleIndex + 1,
            rightIndex
        );
    }

    return recursiveAccountSearch(
        transactions,
        count,
        targetAccount,
        leftIndex,
        middleIndex - 1
    );
}

int recursiveBinarySearchByAccount(
    const Transaction* transactions,
    int count,
    const char* targetAccount
)
{
    if (transactions == NULL || count <= 0 || targetAccount == NULL)
    {
        return -1;
    }

    return recursiveAccountSearch(transactions, count, targetAccount, 0, count - 1);
}
