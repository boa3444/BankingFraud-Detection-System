#ifndef TRANSACTION_H
#define TRANSACTION_H

#define TX_ACCOUNT_LEN 64
#define TX_DESC_LEN 256

/*
    Structure representing a single banking transaction
    and its associated fraud metadata.
*/
typedef struct
{
    int transactionId;
    char senderAccount[TX_ACCOUNT_LEN];
    char receiverAccount[TX_ACCOUNT_LEN];
    char deviceId[TX_ACCOUNT_LEN];
    char merchantId[TX_ACCOUNT_LEN];
    double amount;
    char location[TX_ACCOUNT_LEN];
    char description[TX_DESC_LEN];
    int isFraud;
    long long timestamp;
} Transaction;

/*
    Dynamic array of Transaction records.
*/
typedef struct
{
    Transaction* items;
    int count;
    int capacity;
} TransactionList;

/*
    Loads transaction records from the generated CSV file.
    Caller must free the returned list using freeTransactionList().
*/
TransactionList loadTransactionsFromCSV(const char* fileName);

/*
    Frees heap memory owned by a TransactionList and resets its fields.
*/
void freeTransactionList(TransactionList* list);

#endif
