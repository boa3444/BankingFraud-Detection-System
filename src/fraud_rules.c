#include "../include/fraud_rules.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void toLowerCopy(const char* src, char* dest, size_t destSize)
{
    if (destSize == 0)
    {
        return;
    }

    size_t i = 0;
    if (src != NULL)
    {
        while (src[i] != '\0' && i < destSize - 1)
        {
            dest[i] = (char)tolower((unsigned char)src[i]);
            i++;
        }
    }
    dest[i] = '\0';
}

int checkAmountThreshold(
    const Transaction* transaction,
    double highRiskLimit
)
{
    if (transaction == NULL)
    {
        return 0;
    }
    return transaction->amount > highRiskLimit;
}

int findSuspiciousKeywords(
    const Transaction* transaction,
    char outKeywords[][32],
    int maxOut
)
{
    if (transaction == NULL || maxOut <= 0)
    {
        return 0;
    }

    char lowerDesc[TX_DESC_LEN];
    toLowerCopy(transaction->description, lowerDesc, sizeof(lowerDesc));

    const char* keywords[] = {"urgent", "mule", "siphon", "transfer"};
    const int keywordCount = 4;
    int found = 0;

    for (int i = 0; i < keywordCount && found < maxOut; i++)
    {
        if (strstr(lowerDesc, keywords[i]) != NULL)
        {
            snprintf(outKeywords[found], 32, "%s", keywords[i]);
            found++;
        }
    }

    return found;
}

TransactionRiskReport evaluateTransactionRisk(
    const Transaction* transaction,
    double highRiskLimit
)
{
    TransactionRiskReport report;
    memset(&report, 0, sizeof(report));

    if (transaction == NULL)
    {
        return report;
    }

    report.transactionId = transaction->transactionId;
    snprintf(report.accountId, sizeof(report.accountId), "%s", transaction->senderAccount);

    if (checkAmountThreshold(transaction, highRiskLimit))
    {
        report.riskScore += 40;
        if (report.reasonCount < MAX_RISK_REASONS)
        {
            snprintf(report.reasons[report.reasonCount++], RISK_REASON_LEN,
                     "Amount is above the high-risk threshold");
        }
    }

    char foundKeywords[4][32];
    int kwCount = findSuspiciousKeywords(transaction, foundKeywords, 4);
    for (int i = 0; i < kwCount; i++)
    {
        report.riskScore += 10;
        if (report.reasonCount < MAX_RISK_REASONS)
        {
            snprintf(report.reasons[report.reasonCount++], RISK_REASON_LEN,
                     "Suspicious keyword found: %s", foundKeywords[i]);
        }
    }

    char lowerLoc[TX_ACCOUNT_LEN];
    toLowerCopy(transaction->location, lowerLoc, sizeof(lowerLoc));
    if (strcmp(lowerLoc, "dubai") == 0 || strcmp(lowerLoc, "london") == 0)
    {
        report.riskScore += 20;
        if (report.reasonCount < MAX_RISK_REASONS)
        {
            snprintf(report.reasons[report.reasonCount++], RISK_REASON_LEN,
                     "Transaction originated from an unusual location");
        }
    }

    report.isHighRisk = (report.riskScore >= 50) ? 1 : 0;
    return report;
}

TransactionRiskReportList evaluateAllTransactions(
    const Transaction* transactions,
    int count,
    double highRiskLimit
)
{
    TransactionRiskReportList list = {NULL, 0, 0};
    if (transactions == NULL || count <= 0)
    {
        return list;
    }

    list.items = (TransactionRiskReport*)malloc((size_t)count * sizeof(TransactionRiskReport));
    if (list.items == NULL)
    {
        return list;
    }
    list.capacity = count;

    for (int i = 0; i < count; i++)
    {
        list.items[list.count++] = evaluateTransactionRisk(&transactions[i], highRiskLimit);
    }

    return list;
}

static int findAccountReport(const AccountRiskReportList* list, const char* accountId)
{
    for (int i = 0; i < list->count; i++)
    {
        if (strcmp(list->items[i].accountId, accountId) == 0)
        {
            return i;
        }
    }
    return -1;
}

static void addSuspiciousTxId(AccountRiskReport* acc, int txId)
{
    if (acc->suspiciousIdCount >= acc->suspiciousIdCapacity)
    {
        int newCap = (acc->suspiciousIdCapacity == 0) ? 4 : acc->suspiciousIdCapacity * 2;
        int* resized = (int*)realloc(acc->suspiciousTransactionIds, (size_t)newCap * sizeof(int));
        if (resized == NULL)
        {
            return;
        }
        acc->suspiciousTransactionIds = resized;
        acc->suspiciousIdCapacity = newCap;
    }
    acc->suspiciousTransactionIds[acc->suspiciousIdCount++] = txId;
}

AccountRiskReportList createAccountRiskReports(
    const Transaction* transactions,
    int count,
    double highRiskLimit
)
{
    AccountRiskReportList list = {NULL, 0, 0};
    if (transactions == NULL || count <= 0)
    {
        return list;
    }

    for (int i = 0; i < count; i++)
    {
        TransactionRiskReport txRep = evaluateTransactionRisk(&transactions[i], highRiskLimit);
        int idx = findAccountReport(&list, transactions[i].senderAccount);

        if (idx == -1)
        {
            if (list.count >= list.capacity)
            {
                int newCap = (list.capacity == 0) ? 16 : list.capacity * 2;
                AccountRiskReport* resized = (AccountRiskReport*)realloc(
                    list.items,
                    (size_t)newCap * sizeof(AccountRiskReport)
                );
                if (resized == NULL)
                {
                    break;
                }
                list.items = resized;
                list.capacity = newCap;
            }
            idx = list.count++;
            memset(&list.items[idx], 0, sizeof(AccountRiskReport));
            snprintf(list.items[idx].accountId, TX_ACCOUNT_LEN, "%s", transactions[i].senderAccount);
        }

        list.items[idx].totalRiskScore += txRep.riskScore;
        if (txRep.isHighRisk)
        {
            list.items[idx].suspiciousTransactionCount++;
            addSuspiciousTxId(&list.items[idx], transactions[i].transactionId);
        }
    }

    for (int i = 0; i < list.count; i++)
    {
        if (list.items[i].totalRiskScore >= 70 ||
            list.items[i].suspiciousTransactionCount >= 2)
        {
            list.items[i].isHighRiskAccount = 1;
        }
    }

    return list;
}

void displayTransactionRiskReport(const TransactionRiskReport* report)
{
    if (report == NULL)
    {
        return;
    }

    printf("\nTransaction ID: %d\n", report->transactionId);
    printf("Sender Account: %s\n", report->accountId);
    printf("Risk Score: %d\n", report->riskScore);
    printf("Status: %s\n", report->isHighRisk ? "HIGH RISK" : "Normal");
    for (int i = 0; i < report->reasonCount; i++)
    {
        printf("  - %s\n", report->reasons[i]);
    }
}

void displayAllTransactionRiskReports(const TransactionRiskReportList* reports)
{
    if (reports == NULL)
    {
        return;
    }
    for (int i = 0; i < reports->count; i++)
    {
        displayTransactionRiskReport(&reports->items[i]);
    }
}

void displayAllAccountRiskReports(const AccountRiskReportList* reports)
{
    if (reports == NULL)
    {
        return;
    }
    for (int i = 0; i < reports->count; i++)
    {
        const AccountRiskReport* acc = &reports->items[i];
        printf("\nAccount: %s | Total Score: %d | Suspicious Tx: %d | Status: %s\n",
               acc->accountId,
               acc->totalRiskScore,
               acc->suspiciousTransactionCount,
               acc->isHighRiskAccount ? "HIGH RISK ACCOUNT" : "Normal");
    }
}

void freeTransactionRiskReportList(TransactionRiskReportList* list)
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

void freeAccountRiskReportList(AccountRiskReportList* list)
{
    if (list == NULL)
    {
        return;
    }
    for (int i = 0; i < list->count; i++)
    {
        free(list->items[i].suspiciousTransactionIds);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}
