#include "../include/backtracking.h"
#include "../include/benchmark.h"
#include "../include/data_generator.h"
#include "../include/divide_conquer.h"
#include "../include/dp.h"
#include "../include/flow.h"
#include "../include/fraud_rules.h"
#include "../include/graph.h"
#include "../include/greedy.h"
#include "../include/number_theory.h"
#include "../include/searching.h"
#include "../include/sorting.h"
#include "../include/string_matching.h"
#include "../include/transaction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void displayTransactions(const Transaction* transactions, int count)
{
    printf("\n----------------- TRANSACTIONS ----------------\n");
    for (int i = 0; i < count; i++)
    {
        printf("\nTransaction ID: %d\n", transactions[i].transactionId);
        printf("Sender Account: %s\n", transactions[i].senderAccount);
        printf("Receiver Account: %s\n", transactions[i].receiverAccount);
        printf("Amount: Rs. %.2f\n", transactions[i].amount);
        printf("Description: %s\n", transactions[i].description);
    }
}

static void displayMenu(void)
{
    printf("\n---------------------------------------------------\n");
    printf(" BANKING FRAUD INVESTIGATION WORKBENCH (C17)\n");
    printf("---------------------------------------------------\n");
    printf("[1]  Display Fraud Investigation Graph\n");
    printf("[2]  Detect Money-Laundering Rings (DFS)\n");
    printf("[3]  Run Rule-Based Fraud Scanning\n");
    printf("[4]  Sort Transactions (Merge / Quick Sort)\n");
    printf("[5]  Search Transactions (Binary Search)\n");
    printf("[6]  Scan Transaction Logs (String Matching)\n");
    printf("[7]  Run Complete Performance Benchmark (+ CSV)\n");
    printf("[8]  Run DP & Backtracking / Branch-and-Bound\n");
    printf("[9]  Run Max Flow, Bipartite Matching & CRT\n");
    printf("[10] Run Project 07 Three Demo Scenarios\n");
    printf("[11] Exit Workbench\n");
    printf("---------------------------------------------------\n");
}

static void runSortingMenu(const Transaction* transactions, int count)
{
    Transaction* copy = (Transaction*)malloc((size_t)count * sizeof(Transaction));
    if (copy == NULL) return;
    memcpy(copy, transactions, (size_t)count * sizeof(Transaction));

    int choice = 0;
    printf("\n[1] Merge Sort by Amount\n[2] Quick Sort by Amount\n");
    printf("[3] Merge Sort by ID\n[4] Quick Sort by ID\nChoose: ");
    if (scanf("%d", &choice) == 1)
    {
        SortingStatistics st = {0, 0, 0};
        if (choice == 1) { mergeSortByAmount(copy, count, &st); displaySortingStatistics("Merge Sort (Amount)", st); }
        else if (choice == 2) { quickSortByAmount(copy, count, &st); displaySortingStatistics("Quick Sort (Amount)", st); }
        else if (choice == 3) { mergeSortByTransactionId(copy, count, &st); displaySortingStatistics("Merge Sort (ID)", st); }
        else if (choice == 4) { quickSortByTransactionId(copy, count, &st); displaySortingStatistics("Quick Sort (ID)", st); }
        displayTransactions(copy, count < 5 ? count : 5);
    }
    free(copy);
}

static void runSearchingMenu(const Transaction* transactions, int count)
{
    Transaction* copy = (Transaction*)malloc((size_t)count * sizeof(Transaction));
    if (copy == NULL) return;
    memcpy(copy, transactions, (size_t)count * sizeof(Transaction));

    SortingStatistics st = {0, 0, 0};
    mergeSortByTransactionId(copy, count, &st);

    int targetId = 0;
    printf("\nEnter Transaction ID to search: ");
    if (scanf("%d", &targetId) == 1)
    {
        int idxIter = iterativeBinarySearchById(copy, count, targetId);
        int idxRec = recursiveBinarySearchById(copy, count, targetId);
        if (idxIter >= 0)
        {
            printf("Found at sorted index %d (recursive=%d): %s -> %s (Rs. %.2f)\n",
                   idxIter, idxRec, copy[idxIter].senderAccount,
                   copy[idxIter].receiverAccount, copy[idxIter].amount);
        }
        else
        {
            printf("Transaction ID %d not found.\n", targetId);
        }
    }
    free(copy);
}

static void runStringMatchingMenu(const Transaction* transactions, int count)
{
    char pattern[128];
    printf("\nEnter keyword to search in descriptions: ");
    if (scanf("%127s", pattern) == 1)
    {
        runStringMatchingBenchmark(transactions, count, pattern);
    }
}

static void runFraudRulesMenu(const Transaction* transactions, int count)
{
    double limit = 20000.0;
    printf("\nEnter high-risk amount threshold (e.g., 20000): ");
    if (scanf("%lf", &limit) != 1) limit = 20000.0;

    TransactionRiskReportList txReports = evaluateAllTransactions(transactions, count, limit);
    displayAllTransactionRiskReports(&txReports);
    freeTransactionRiskReportList(&txReports);

    AccountRiskReportList accReports = createAccountRiskReports(transactions, count, limit);
    displayAllAccountRiskReports(&accReports);
    freeAccountRiskReportList(&accReports);
}

static void runRingDetectionMenu(FraudGraph* graph, const Transaction* transactions, int count)
{
    LaunderingRingList rings = detectAllRings(graph, transactions, count);
    if (rings.count == 0)
    {
        printf("\nNo money-laundering rings detected.\n");
    }
    else
    {
        printf("\nDetected %d laundering ring(s):\n", rings.count);
        for (int i = 0; i < rings.count; i++)
        {
            printf("  Start: %s | Total Amount: Rs. %.2f | Path: ",
                   rings.items[i].startingAccount, rings.items[i].totalAmount);
            for (int j = 0; j < rings.items[i].pathLength; j++)
            {
                printf("%s%s", rings.items[i].path[j],
                       (j < rings.items[i].pathLength - 1) ? " -> " : "\n");
            }
        }
    }
    freeLaunderingRingList(&rings);
}

static void runDpAndExactMenu(void)
{
    printf("\n---------- DP, BACKTRACKING & BRANCH-AND-BOUND ----------\n");
    CaseItem cases[4] = {
        {101, 40.0, 2.0},
        {102, 50.0, 5.0},
        {103, 100.0, 10.0},
        {104, 95.0, 5.0}
    };
    Metrics m;

    FractionalKnapsackResult fk = fractionalKnapsack(cases, 4, 16.0, &m);
    printf("Greedy Fractional Knapsack (16h): risk=%.1f (%lld us)\n", fk.totalRisk, m.elapsedMicros);
    freeFractionalKnapsackResult(&fk);

    DpKnapsackResult dp = knapsack01DP(cases, 4, 16, &m);
    printf("DP 0/1 Knapsack (16h):            risk=%.1f (%lld us)\n", dp.maxRisk, m.elapsedMicros);
    freeDpKnapsackResult(&dp);

    ExactKnapsackResult bt = knapsack01Backtracking(cases, 4, 16.0, &m);
    printf("Backtracking 0/1 Knapsack:        risk=%.1f, nodes=%lld\n", bt.maxRisk, bt.nodesExplored);
    freeExactKnapsackResult(&bt);

    ExactKnapsackResult lc = knapsack01BranchAndBound(cases, 4, 16.0, BNB_LEAST_COST, &m);
    printf("Least-Cost Branch & Bound:        risk=%.1f, nodes=%lld\n", lc.maxRisk, lc.nodesExplored);
    freeExactKnapsackResult(&lc);

    LcsResult lcs = longestCommonSubsequence("WIRE_OFFSHORE_URGENT", "ATM_WIRE_URGENT", &m);
    printf("LCS Pattern Similarity:           length=%d (%s)\n", lcs.length, lcs.sequence ? lcs.sequence : "");
    freeLcsResult(&lcs);

    double tspCost[16] = {
        0, 10, 15, 20,
        10, 0, 35, 25,
        15, 35, 0, 30,
        20, 25, 30, 0
    };
    TspResult tsp = tspHeldKarp(tspCost, 4, &m);
    printf("Bounded Audit Tour (TSP, 4 nodes): minCost=%.1f\n", tsp.minCost);
    freeTspResult(&tsp);
}

static void runFlowAndCrtMenu(const Transaction* transactions, int count)
{
    printf("\n---------- MAX FLOW, BIPARTITE MATCHING & CRT ----------\n");
    Metrics m;

    double cap[36] = {
        0, 16, 13, 0, 0, 0,
        0, 0, 10, 12, 0, 0,
        0, 4, 0, 0, 14, 0,
        0, 0, 9, 0, 0, 20,
        0, 0, 0, 7, 0, 4,
        0, 0, 0, 0, 0, 0
    };
    MaxFlowResult mf = fordFulkersonMaxFlow(6, 0, 5, cap, &m);
    printf("Ford-Fulkerson Max Transfer Flow (0 -> 5): %.1f\n", mf.maxFlow);
    freeMaxFlowResult(&mf);

    int adj[9] = {1, 1, 0, 0, 1, 0, 0, 0, 1};
    BipartiteMatchingResult bm = maxBipartiteMatching(3, 3, adj, &m);
    printf("Bipartite Case-to-Investigator Matches: %d / 3\n", bm.maxMatches);
    freeBipartiteMatchingResult(&bm);

    long long mods[3] = {101, 103, 107};
    CrtResult crt = reconstructDistributedTxTotal(transactions, count, mods, 3, &m);
    printf("CRT Distributed Checksum Total: %lld (mod %lld, valid=%d)\n",
           crt.value, crt.combinedModulus, crt.valid);
}

static void runDemoScenarios(FraudGraph* graph, const Transaction* transactions, int count)
{
    printf("\n============================================================\n");
    printf("        PROJECT 07: THREE DEMONSTRATION SCENARIOS\n");
    printf("============================================================\n");

    /* Scenario 1: Normal Operation */
    printf("\n[Scenario 1: Normal Operation (%d transactions)]\n", count);
    LaunderingRingList rings = detectAllRings(graph, transactions, count);
    AccountRiskReportList accs = createAccountRiskReports(transactions, count, 20000.0);
    int highRiskAccs = 0;
    for (int i = 0; i < accs.count; i++)
    {
        if (accs.items[i].isHighRiskAccount) highRiskAccs++;
    }
    Metrics m;
    MstResult mst = investigationNetwork(transactions, count, 1, &m);
    printf("  Laundering rings detected: %d | High-risk accounts: %d\n", rings.count, highRiskAccs);
    printf("  Kruskal Investigation MST weight: %.2f (%d edges)\n", mst.totalWeight, mst.edgeCount);
    freeLaunderingRingList(&rings);
    freeAccountRiskReportList(&accs);
    freeMstResult(&mst);

    /* Scenario 2: Capacity / Constraint Shock */
    printf("\n[Scenario 2: Capacity Shock (Only 8 investigator hours, 4 urgent cases)]\n");
    CaseItem shockCases[4] = {
        {1, 40.0, 2.0},
        {2, 50.0, 5.0},
        {3, 100.0, 7.0},
        {4, 95.0, 5.0}
    };
    FractionalKnapsackResult fk = fractionalKnapsack(shockCases, 4, 8.0, &m);
    DpKnapsackResult dp = knapsack01DP(shockCases, 4, 8, &m);
    ExactKnapsackResult fifo = knapsack01BranchAndBound(shockCases, 4, 8.0, BNB_FIFO, &m);
    ExactKnapsackResult lc = knapsack01BranchAndBound(shockCases, 4, 8.0, BNB_LEAST_COST, &m);
    printf("  Greedy Fractional: risk=%.1f | DP 0/1 Exact: risk=%.1f\n", fk.totalRisk, dp.maxRisk);
    printf("  BnB FIFO nodes: %lld | BnB Least-Cost nodes: %lld (both risk=%.1f)\n",
           fifo.nodesExplored, lc.nodesExplored, lc.maxRisk);
    freeFractionalKnapsackResult(&fk);
    freeDpKnapsackResult(&dp);
    freeExactKnapsackResult(&fifo);
    freeExactKnapsackResult(&lc);

    /* Scenario 3: Large-Data Stress Case */
    const int stressN = 2000;
    printf("\n[Scenario 3: Large-Data Stress Case (n=%d transactions)]\n", stressN);
    Transaction* stress = (Transaction*)calloc((size_t)stressN, sizeof(Transaction));
    double* buckets = (double*)malloc((size_t)stressN * sizeof(double));
    if (stress && buckets)
    {
        for (int i = 0; i < stressN; i++)
        {
            stress[i].transactionId = stressN - i;
            stress[i].amount = (double)(((i * 37 + 11) % 50000) + 100);
            buckets[i] = ((i % 3 == 0) ? 1.0 : -0.6) * ((i * 19 % 500) + 10);
        }
        Transaction* copy = (Transaction*)malloc((size_t)stressN * sizeof(Transaction));
        if (copy)
        {
            SortingStatistics stM, stQ;
            memcpy(copy, stress, (size_t)stressN * sizeof(Transaction));
            mergeSortByAmount(copy, stressN, &stM);
            memcpy(copy, stress, (size_t)stressN * sizeof(Transaction));
            quickSortByAmount(copy, stressN, &stQ);
            printf("  MergeSort comparisons: %lld | QuickSort comparisons: %lld\n",
                   stM.comparisons, stQ.comparisons);
            free(copy);
        }

        Metrics mDC, mKad;
        SubarrayResult sDC = maxSubarrayDivideConquer(buckets, stressN, &mDC);
        SubarrayResult sKad = maxSubarrayKadane(buckets, stressN, &mKad);
        printf("  MaxSubarray D&C: sum=%.1f (%lld us, %lld comps) | Kadane: sum=%.1f (%lld us, %lld comps)\n",
               sDC.sum, mDC.elapsedMicros, mDC.comparisons,
               sKad.sum, mKad.elapsedMicros, mKad.comparisons);
    }
    free(stress);
    free(buckets);
}

int main(void)
{
    const char* dataFile = "data/transactions.csv";
    generateTransactionCSV(dataFile);

    TransactionList list = loadTransactionsFromCSV(dataFile);
    if (list.count == 0)
    {
        printf("No transactions loaded.\n");
        freeTransactionList(&list);
        return 1;
    }

    FraudGraph graph;
    initFraudGraph(&graph);
    buildFraudGraph(&graph, list.items, list.count);

    int choice = 0;
    while (1)
    {
        displayMenu();
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            break;
        }

        if (choice == 1) displayFraudGraph(&graph);
        else if (choice == 2) runRingDetectionMenu(&graph, list.items, list.count);
        else if (choice == 3) runFraudRulesMenu(list.items, list.count);
        else if (choice == 4) runSortingMenu(list.items, list.count);
        else if (choice == 5) runSearchingMenu(list.items, list.count);
        else if (choice == 6) runStringMatchingMenu(list.items, list.count);
        else if (choice == 7) runCompleteBenchmark(list.items, list.count);
        else if (choice == 8) runDpAndExactMenu();
        else if (choice == 9) runFlowAndCrtMenu(list.items, list.count);
        else if (choice == 10) runDemoScenarios(&graph, list.items, list.count);
        else if (choice == 11) break;
        else printf("Invalid choice.\n");
    }

    clearFraudGraph(&graph);
    freeTransactionList(&list);
    return 0;
}
