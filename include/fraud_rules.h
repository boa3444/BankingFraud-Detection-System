#ifndef FRAUD_RULES_H
#define FRAUD_RULES_H

#include "transaction.h"

#define MAX_RISK_REASONS 8
#define RISK_REASON_LEN 128

/*
    Stores the result of checking one transaction.
*/
typedef struct
{
    int transactionId;
    char accountId[TX_ACCOUNT_LEN];
    int riskScore;
    int isHighRisk;
    char reasons[MAX_RISK_REASONS][RISK_REASON_LEN];
    int reasonCount;
} TransactionRiskReport;

typedef struct
{
    TransactionRiskReport* items;
    int count;
    int capacity;
} TransactionRiskReportList;

/*
    Stores the combined risk connected with one account.
*/
typedef struct
{
    char accountId[TX_ACCOUNT_LEN];
    int totalRiskScore;
    int suspiciousTransactionCount;
    int isHighRiskAccount;
    int* suspiciousTransactionIds;
    int suspiciousIdCount;
    int suspiciousIdCapacity;
} AccountRiskReport;

typedef struct
{
    AccountRiskReport* items;
    int count;
    int capacity;
} AccountRiskReportList;

/*
    Checks whether the amount is above the high-risk limit.
*/
int checkAmountThreshold(
    const Transaction* transaction,
    double highRiskLimit
);

/*
    Finds suspicious keywords in transaction.description.
    Writes up to maxOut matching keywords into outKeywords and returns the count.
*/
int findSuspiciousKeywords(
    const Transaction* transaction,
    char outKeywords[][32],
    int maxOut
);

/*
    Creates a complete risk report for one transaction.
*/
TransactionRiskReport evaluateTransactionRisk(
    const Transaction* transaction,
    double highRiskLimit
);

/*
    Evaluates every transaction in the array.
    Caller must free with freeTransactionRiskReportList().
*/
TransactionRiskReportList evaluateAllTransactions(
    const Transaction* transactions,
    int count,
    double highRiskLimit
);

/*
    Combines transaction-level risk into account-level risk.
    Caller must free with freeAccountRiskReportList().
*/
AccountRiskReportList createAccountRiskReports(
    const Transaction* transactions,
    int count,
    double highRiskLimit
);

/*
    Display helpers.
*/
void displayTransactionRiskReport(const TransactionRiskReport* report);
void displayAllTransactionRiskReports(const TransactionRiskReportList* reports);
void displayAllAccountRiskReports(const AccountRiskReportList* reports);

/*
    Memory cleanup helpers.
*/
void freeTransactionRiskReportList(TransactionRiskReportList* list);
void freeAccountRiskReportList(AccountRiskReportList* list);

#endif
