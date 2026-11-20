#include "../include/transaction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Copies a bounded substring into dest and strips trailing '\r' or '\n'.
*/
static void copyField(char* dest, size_t destSize, const char* start, size_t len)
{
    if (destSize == 0)
    {
        return;
    }

    size_t copyLen = (len < destSize - 1) ? len : (destSize - 1);
    memcpy(dest, start, copyLen);
    dest[copyLen] = '\0';

    while (copyLen > 0 && (dest[copyLen - 1] == '\r' || dest[copyLen - 1] == '\n'))
    {
        dest[--copyLen] = '\0';
    }
}

/*
    Reads the next comma-separated field starting at *cursor.
    Returns 1 if a field was read, 0 if *cursor is already at end of line.
*/
static int nextCsvField(const char** cursor, char* out, size_t outSize)
{
    const char* p = *cursor;
    if (p == NULL || *p == '\0' || *p == '\n' || *p == '\r')
    {
        return 0;
    }

    const char* comma = strchr(p, ',');
    if (comma != NULL)
    {
        copyField(out, outSize, p, (size_t)(comma - p));
        *cursor = comma + 1;
    }
    else
    {
        size_t len = strlen(p);
        copyField(out, outSize, p, len);
        *cursor = p + len;
    }

    return 1;
}

static int appendTransaction(TransactionList* list, const Transaction* tx)
{
    if (list->count >= list->capacity)
    {
        int newCapacity = (list->capacity == 0) ? 16 : list->capacity * 2;
        Transaction* resized = (Transaction*)realloc(
            list->items,
            (size_t)newCapacity * sizeof(Transaction)
        );
        if (resized == NULL)
        {
            return 0;
        }
        list->items = resized;
        list->capacity = newCapacity;
    }

    list->items[list->count++] = *tx;
    return 1;
}

TransactionList loadTransactionsFromCSV(const char* fileName)
{
    TransactionList list = {NULL, 0, 0};

    FILE* file = fopen(fileName, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Could not open transaction file: %s\n", fileName);
        return list;
    }

    char line[1024];

    /* Skip the CSV header row */
    if (fgets(line, sizeof(line), file) == NULL)
    {
        fclose(file);
        return list;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        const char* cursor = line;
        char idBuf[64], amountBuf[64], fraudBuf[64], tsBuf[64];
        Transaction tx;
        memset(&tx, 0, sizeof(tx));

        if (!nextCsvField(&cursor, idBuf, sizeof(idBuf)) ||
            !nextCsvField(&cursor, tx.senderAccount, sizeof(tx.senderAccount)) ||
            !nextCsvField(&cursor, tx.receiverAccount, sizeof(tx.receiverAccount)) ||
            !nextCsvField(&cursor, tx.deviceId, sizeof(tx.deviceId)) ||
            !nextCsvField(&cursor, tx.merchantId, sizeof(tx.merchantId)) ||
            !nextCsvField(&cursor, amountBuf, sizeof(amountBuf)))
        {
            continue;
        }

        tx.transactionId = atoi(idBuf);
        tx.amount = strtod(amountBuf, NULL);

        nextCsvField(&cursor, tx.location, sizeof(tx.location));
        nextCsvField(&cursor, tx.description, sizeof(tx.description));

        if (nextCsvField(&cursor, fraudBuf, sizeof(fraudBuf)))
        {
            tx.isFraud = atoi(fraudBuf);
        }

        if (nextCsvField(&cursor, tsBuf, sizeof(tsBuf)))
        {
            tx.timestamp = strtoll(tsBuf, NULL, 10);
        }

        if (!appendTransaction(&list, &tx))
        {
            fprintf(stderr, "Error: Out of memory while loading transactions.\n");
            break;
        }
    }

    fclose(file);
    return list;
}

void freeTransactionList(TransactionList* list)
{
    if (list == NULL)
    {
        return;
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}
