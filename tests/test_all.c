#include "../include/analysis.h"
#include "../include/backtracking.h"
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

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void testSmallInstanceOracles(void)
{
    Metrics m;

    /* Sorting & Searching oracle */
    Transaction txs[4] = {
        {30, "A3", "B", "D", "M", 500.0, "Delhi", "x", 0, 10},
        {10, "A1", "B", "D", "M", 100.0, "Delhi", "x", 0, 10},
        {40, "A4", "B", "D", "M", 900.0, "Delhi", "x", 1, 10},
        {20, "A2", "B", "D", "M", 250.0, "Delhi", "x", 0, 10}
    };
    SortingStatistics st;
    mergeSortByTransactionId(txs, 4, &st);
    assert(txs[0].transactionId == 10 && txs[3].transactionId == 40);
    assert(iterativeBinarySearchById(txs, 4, 30) == 2);
    assert(recursiveBinarySearchById(txs, 4, 30) == 2);

    /* String Matching oracle (all 4 must match identical positions) */
    const char* text = "urgent transfer and transfer alert";
    StringMatchResult sm1 = naiveStringMatching(text, "transfer");
    StringMatchResult sm2 = rabinKarpStringMatching(text, "transfer");
    StringMatchResult sm3 = finiteAutomatonMatching(text, "transfer");
    StringMatchResult sm4 = kmpStringMatching(text, "transfer");
    assert(sm1.matchCount == 2 && sm2.matchCount == 2 && sm3.matchCount == 2 && sm4.matchCount == 2);
    assert(sm1.matchingPositions[0] == 7 && sm4.matchingPositions[1] == 20);
    freeStringMatchResult(&sm1); freeStringMatchResult(&sm2);
    freeStringMatchResult(&sm3); freeStringMatchResult(&sm4);

    /* Divide & Conquer vs Baselines */
    double vals[6] = {-2.0, 5.0, -1.0, 4.0, -10.0, 3.0};
    SubarrayResult sDC = maxSubarrayDivideConquer(vals, 6, &m);
    SubarrayResult sKad = maxSubarrayKadane(vals, 6, &m);
    assert(sDC.valid && sKad.valid && fabs(sDC.sum - 8.0) < 1e-6 && fabs(sKad.sum - 8.0) < 1e-6);

    int knows[9] = {0, 1, 0, 0, 0, 0, 0, 1, 0};
    CelebrityResult c1 = celebrityElimination(knows, 3, &m);
    CelebrityResult c2 = celebrityBruteForce(knows, 3, &m);
    assert(c1.celebrity == 1 && c2.celebrity == 1);

    Matrix a = createMatrix(20, 20), b = createMatrix(20, 20);
    for (int i = 0; i < 400; i++) { a.data[i] = (i % 5) + 1; b.data[i] = (i % 3) + 2; }
    MatrixComparison cmp = compareMatrixMultiplication(&a, &b);
    assert(cmp.equal == 1);
    freeMatrix(&a); freeMatrix(&b);

    /* Greedy MST oracle */
    WeightedEdge edges[5] = {{0, 1, 4.0}, {0, 2, 1.0}, {1, 2, 2.0}, {1, 3, 5.0}, {2, 3, 3.0}};
    MstResult mk = kruskalMst(4, edges, 5, &m);
    MstResult mp = primMst(4, edges, 5, &m);
    assert(compareMstTotals(&mk, &mp) == 1 && fabs(mk.totalWeight - 6.0) < 1e-6);
    freeMstResult(&mk); freeMstResult(&mp);

    /* DP vs Backtracking vs FIFO/LIFO/Least-Cost B&B 0/1 Knapsack oracle */
    CaseItem items[4] = {{1, 40.0, 2.0}, {2, 50.0, 5.0}, {3, 100.0, 10.0}, {4, 95.0, 5.0}};
    DpKnapsackResult kdp = knapsack01DP(items, 4, 16, &m);
    ExactKnapsackResult kbt = knapsack01Backtracking(items, 4, 16.0, &m);
    ExactKnapsackResult kfifo = knapsack01BranchAndBound(items, 4, 16.0, BNB_FIFO, &m);
    ExactKnapsackResult klifo = knapsack01BranchAndBound(items, 4, 16.0, BNB_LIFO, &m);
    ExactKnapsackResult klc = knapsack01BranchAndBound(items, 4, 16.0, BNB_LEAST_COST, &m);
    assert(fabs(kdp.maxRisk - 195.0) < 1e-6);
    assert(fabs(kbt.maxRisk - 195.0) < 1e-6);
    assert(fabs(kfifo.maxRisk - 195.0) < 1e-6);
    assert(fabs(klifo.maxRisk - 195.0) < 1e-6);
    assert(fabs(klc.maxRisk - 195.0) < 1e-6);
    freeDpKnapsackResult(&kdp); freeExactKnapsackResult(&kbt);
    freeExactKnapsackResult(&kfifo); freeExactKnapsackResult(&klifo);
    freeExactKnapsackResult(&klc);

    /* Flow & Matching oracle */
    double cap[36] = {
        0, 16, 13, 0, 0, 0,
        0, 0, 10, 12, 0, 0,
        0, 4, 0, 0, 14, 0,
        0, 0, 9, 0, 0, 20,
        0, 0, 0, 7, 0, 4,
        0, 0, 0, 0, 0, 0
    };
    MaxFlowResult mf = fordFulkersonMaxFlow(6, 0, 5, cap, &m);
    assert(fabs(mf.maxFlow - 23.0) < 1e-6);
    freeMaxFlowResult(&mf);

    int adj[9] = {1, 1, 0, 0, 1, 0, 0, 0, 1};
    BipartiteMatchingResult bm = maxBipartiteMatching(3, 3, adj, &m);
    assert(bm.maxMatches == 3);
    freeBipartiteMatchingResult(&bm);

    /* CRT oracle */
    long long rems[3] = {2, 3, 2}, mods[3] = {3, 5, 7};
    CrtResult crt = chineseRemainderTheorem(rems, mods, 3, &m);
    assert(crt.valid == 1 && crt.value == 23 && crt.combinedModulus == 105);
}

static void testAdditionalUnits(void)
{
    Metrics m;

    assert(modPowIterative(2, 10, 1000, &m) == 24);
    assert(modPowRecursive(2, 10, 1000, &m) == 24);
    assert(naiveModPow(2, 10, 1000, &m) == 24);

    int dims[4] = {10, 30, 5, 60};
    assert(matrixChainOrder(dims, 4, &m).minOperations == 4500);

    LcsResult lcs = longestCommonSubsequence("AGGTAB", "GXTXAYB", &m);
    assert(lcs.length == 4 && strcmp(lcs.sequence, "GTAB") == 0);
    freeLcsResult(&lcs);

    double freq[3] = {0.5, 0.1, 0.4};
    assert(fabs(optimalBST(freq, 3, &m).minCost - 1.6) < 1e-6);

    double tspCost[16] = {0,10,15,20, 10,0,35,25, 15,35,0,30, 20,25,30,0};
    TspResult tsp = tspHeldKarp(tspCost, 4, &m);
    assert(tsp.feasible == 1 && fabs(tsp.minCost - 80.0) < 1e-6);
    freeTspResult(&tsp);

    NQueensResult q4 = solveNQueens(4, &m);
    assert(q4.solutionCount == 2);
    freeNQueensResult(&q4);

    double subVals[4] = {10.0, 20.0, 30.0, 40.0};
    SubsetSumResult ss = sumOfSubsets(subVals, 4, 50.0, &m);
    assert(ss.found == 1 && ss.solutionCount == 2);
    freeSubsetSumResult(&ss);

    assert(huffmanRoundTrip("bank_fraud_compression_test", &m) == 1);
}

static void testIntegrationAndRegression(void)
{
    const char* path = "data/test_regression.csv";
    remove(path);
    assert(generateTransactionCSV(path) == 1);

    TransactionList list = loadTransactionsFromCSV(path);
    assert(list.count == 8);

    FraudGraph g;
    initFraudGraph(&g);
    buildFraudGraph(&g, list.items, list.count);

    /* Regression: cycle A1001 -> A1002 -> A1003 -> A1001 with 25k+24k+23k = 72000 */
    LaunderingRing ring = detectRingFromAccount(&g, "A1001", list.items, list.count);
    assert(ring.ringFound == 1 && fabs(ring.totalAmount - 72000.0) < 1e-6);
    freeLaunderingRing(&ring);

    AccountRiskReportList accs = createAccountRiskReports(list.items, list.count, 20000.0);
    assert(accs.count > 0);
    freeAccountRiskReportList(&accs);

    Metrics m;
    MstResult net = investigationNetwork(list.items, list.count, 1, &m);
    assert(net.edgeCount > 0);
    freeMstResult(&net);

    long long shardMods[3] = {101, 103, 107};
    CrtResult crt = reconstructDistributedTxTotal(list.items, list.count, shardMods, 3, &m);
    assert(crt.valid == 1 && crt.value == 78600LL);

    clearFraudGraph(&g);
    freeTransactionList(&list);
    remove(path);
}

static void testEdgeAndInvalidInputs(void)
{
    Metrics m;

    /* Empty / single / missing */
    assert(iterativeBinarySearchById(NULL, 0, 10) == -1);
    assert(recursiveBinarySearchByAccount(NULL, 0, "A1") == -1);
    assert(huffmanRoundTrip("", &m) == 1);
    assert(huffmanRoundTrip("aaaaaa", &m) == 1);

    TransactionList missing = loadTransactionsFromCSV("data/does_not_exist_xyz.csv");
    assert(missing.count == 0 && missing.items == NULL);
    freeTransactionList(&missing);

    /* Invalid capacities, dimensions, CRT */
    assert(fractionalKnapsack(NULL, 0, -5.0, &m).count == 0);
    assert(fordFulkersonMaxFlow(3, 0, 0, NULL, &m).maxFlow == 0.0);

    int exists = 1;
    assert(modInverse(2, 4, &exists) == 0 && exists == 0);
    long long badRems[2] = {1, 2}, badMods[2] = {2, 4};
    assert(chineseRemainderTheorem(badRems, badMods, 2, &m).valid == 0);
}



/* ============================================================
   RANDOMIZED PROPERTY, MALFORMED INPUT, AND STRESS TESTS
   ============================================================ */

static void testRandomizedProperties(void)
{
    srand(42);
    for (int trial = 0; trial < 10; trial++)
    {
        int n = 50 + (rand() % 150);
        Transaction* arr1 = (Transaction*)calloc((size_t)n, sizeof(Transaction));
        Transaction* arr2 = (Transaction*)calloc((size_t)n, sizeof(Transaction));
        for (int i = 0; i < n; i++)
        {
            arr1[i].transactionId = (rand() % 10000) + 1;
            arr1[i].amount = (double)(rand() % 100000) / 3.0;
        }
        memcpy(arr2, arr1, (size_t)n * sizeof(Transaction));

        SortingStatistics s1, s2;
        mergeSortByAmount(arr1, n, &s1);
        quickSortByAmount(arr2, n, &s2);
        for (int i = 1; i < n; i++)
        {
            assert(arr1[i - 1].amount <= arr1[i].amount);
            assert(arr2[i - 1].amount <= arr2[i].amount);
        }

        mergeSortByTransactionId(arr1, n, &s1);
        for (int i = 1; i < n; i++)
        {
            assert(arr1[i - 1].transactionId <= arr1[i].transactionId);
        }

        int key = arr1[rand() % n].transactionId;
        int idx1 = iterativeBinarySearchById(arr1, n, key);
        int idx2 = recursiveBinarySearchById(arr1, n, key);
        assert(idx1 >= 0 && arr1[idx1].transactionId == key);
        assert(idx2 >= 0 && arr1[idx2].transactionId == key);

        free(arr1);
        free(arr2);
    }
}

static void testMalformedCsvAndGraphEdgeCases(void)
{
    Metrics m;

    /* Malformed CSV with incomplete rows and non-numeric values */
    const char* badCsv = "data/malformed_test.csv";
    FILE* f = fopen(badCsv, "w");
    assert(f != NULL);
    fprintf(f, "transactionId,senderAccount,receiverAccount,deviceId,merchantId,amount,location,description,isFraud,timestamp\n");
    fprintf(f, "bad_row_only_two_cols,A1\n");
    fprintf(f, "10,A1,A2,D1,M1,500.0,Delhi,ok,0,1000\n");
    fclose(f);

    TransactionList parsed = loadTransactionsFromCSV(badCsv);
    assert(parsed.count == 1 && parsed.items[0].transactionId == 10);
    freeTransactionList(&parsed);
    remove(badCsv);

    /* Disconnected graph + duplicate edges in MST */
    WeightedEdge discEdges[3] = {
        {0, 1, 5.0},
        {0, 1, 2.0}, /* duplicate cheaper edge */
        {2, 3, 4.0}
    };
    MstResult mk = kruskalMst(4, discEdges, 3, &m);
    MstResult mp = primMst(4, discEdges, 3, &m);
    assert(mk.componentCount == 2 && mp.componentCount == 2);
    assert(compareMstTotals(&mk, &mp) == 1 && fabs(mk.totalWeight - 6.0) < 1e-6);
    freeMstResult(&mk);
    freeMstResult(&mp);

    /* Negative-weight & negative-cycle detection in Bellman-Ford & Floyd-Warshall */
    WeightedGraphData negCycle;
    initWeightedGraph(&negCycle);
    getOrAddAccountIndex(&negCycle.accounts, "N0");
    getOrAddAccountIndex(&negCycle.accounts, "N1");
    getOrAddAccountIndex(&negCycle.accounts, "N2");
    addWeightedEdge(&negCycle, 0, 1, 1.0);
    addWeightedEdge(&negCycle, 1, 2, -3.0);
    addWeightedEdge(&negCycle, 2, 0, 1.0); /* cycle total = -1 */

    BellmanFordResult bf = bellmanFord(&negCycle, 0, &m);
    assert(bf.hasNegativeCycle == 1);
    freeBellmanFordResult(&bf);

    FloydWarshallResult fw = floydWarshall(&negCycle, &m);
    assert(fw.hasNegativeCycle == 1);
    freeFloydWarshallResult(&fw);
    freeWeightedGraph(&negCycle);
}

static void testLargeStress10k(void)
{
    const int n = 10000;
    Transaction* arr = (Transaction*)calloc((size_t)n, sizeof(Transaction));
    assert(arr != NULL);
    for (int i = 0; i < n; i++)
    {
        arr[i].transactionId = n - i;
        arr[i].amount = (double)((i * 97) % 50000);
    }
    SortingStatistics st;
    mergeSortByTransactionId(arr, n, &st);
    assert(arr[0].transactionId == 1 && arr[n - 1].transactionId == n);
    assert(iterativeBinarySearchById(arr, n, 7777) == 7776);
    free(arr);
}

int main(void)
{
    testSmallInstanceOracles();
    testAdditionalUnits();
    testIntegrationAndRegression();
    testEdgeAndInvalidInputs();
    testRandomizedProperties();
    testMalformedCsvAndGraphEdgeCases();
    testLargeStress10k();
    printf("ALL_C17_PROJECT07_TESTS_PASSED\n");
    return 0;
}
