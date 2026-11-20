#include "../include/benchmark.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* createTransactionLog(const Transaction* transactions, int count)
{
    if (transactions == NULL || count <= 0)
    {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    size_t cap = (size_t)count * (TX_DESC_LEN + 2) + 1;
    char* log = (char*)malloc(cap);
    if (log == NULL) return NULL;
    size_t pos = 0;

    for (int i = 0; i < count; i++)
    {
        int n = snprintf(log + pos, cap - pos, "%s ", transactions[i].description);
        if (n > 0) pos += (size_t)n;
    }
    return log;
}

static long long nowNanos(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long long)ts.tv_sec * 1000000000LL + (long long)ts.tv_nsec;
}

void runSortingBenchmark(
    const Transaction* transactions,
    int count
)
{
    printf("\n                 # SORTING BENCHMARK\n");
    if (transactions == NULL || count <= 0)
    {
        printf("No transactions to benchmark.\n");
        return;
    }

    Transaction* mergeData = (Transaction*)malloc((size_t)count * sizeof(Transaction));
    Transaction* quickData = (Transaction*)malloc((size_t)count * sizeof(Transaction));
    if (!mergeData || !quickData)
    {
        free(mergeData);
        free(quickData);
        return;
    }
    memcpy(mergeData, transactions, (size_t)count * sizeof(Transaction));
    memcpy(quickData, transactions, (size_t)count * sizeof(Transaction));

    SortingStatistics mergeStats = {0, 0, 0};
    SortingStatistics quickStats = {0, 0, 0};

    long long t0 = nowNanos();
    mergeSortByAmount(mergeData, count, &mergeStats);
    long long mergeNs = nowNanos() - t0;

    t0 = nowNanos();
    quickSortByAmount(quickData, count, &quickStats);
    long long quickNs = nowNanos() - t0;

    printf("+--------------+--------------+--------------+------------+----------------+\n");
    printf("|%-14s|%-14s|%-14s|%-12s|%-16s|\n",
           "Algorithm", "Time (ns)", "Comparisons", "Swaps", "Recursive Calls");
    printf("+--------------+--------------+--------------+------------+----------------+\n");
    printf("|%-14s|%-14lld|%-14lld|%-12lld|%-16lld|\n",
           "Merge Sort", mergeNs, mergeStats.comparisons, mergeStats.swaps, mergeStats.recursiveCalls);
    printf("|%-14s|%-14lld|%-14lld|%-12lld|%-16lld|\n",
           "Quick Sort", quickNs, quickStats.comparisons, quickStats.swaps, quickStats.recursiveCalls);
    printf("+--------------+--------------+--------------+------------+----------------+\n");
    printf("Transactions tested: %d\n", count);

    free(mergeData);
    free(quickData);
}

void runStringMatchingBenchmark(
    const Transaction* transactions,
    int count,
    const char* pattern
)
{
    printf("\n             # STRING MATCHING BENCHMARK\n");
    char* log = createTransactionLog(transactions, count);
    if (log == NULL) return;

    long long t0 = nowNanos();
    StringMatchResult naiveRes = naiveStringMatching(log, pattern);
    long long naiveNs = nowNanos() - t0;

    t0 = nowNanos();
    StringMatchResult rkRes = rabinKarpStringMatching(log, pattern);
    long long rkNs = nowNanos() - t0;

    t0 = nowNanos();
    StringMatchResult faRes = finiteAutomatonMatching(log, pattern);
    long long faNs = nowNanos() - t0;

    t0 = nowNanos();
    StringMatchResult kmpRes = kmpStringMatching(log, pattern);
    long long kmpNs = nowNanos() - t0;

    printf("+------------------+--------------+------------+--------------+------------------+\n");
    printf("|%-18s|%-14s|%-12s|%-14s|%-18s|\n",
           "Algorithm", "Time (ns)", "Matches", "Comparisons", "Extra Operations");
    printf("+------------------+--------------+------------+--------------+------------------+\n");
    printf("|%-18s|%-14lld|%-12d|%-14lld|%-18lld|\n",
           "Naive", naiveNs, naiveRes.matchCount, naiveRes.comparisons,
           naiveRes.hashCalculations + naiveRes.tableOperations);
    printf("|%-18s|%-14lld|%-12d|%-14lld|%-18lld|\n",
           "Rabin-Karp", rkNs, rkRes.matchCount, rkRes.comparisons,
           rkRes.hashCalculations + rkRes.tableOperations);
    printf("|%-18s|%-14lld|%-12d|%-14lld|%-18lld|\n",
           "Finite Automaton", faNs, faRes.matchCount, faRes.comparisons,
           faRes.hashCalculations + faRes.tableOperations);
    printf("|%-18s|%-14lld|%-12d|%-14lld|%-18lld|\n",
           "KMP", kmpNs, kmpRes.matchCount, kmpRes.comparisons,
           kmpRes.hashCalculations + kmpRes.tableOperations);
    printf("+------------------+--------------+------------+--------------+------------------+\n");
    printf("Log length: %zu characters\n", strlen(log));
    printf("Search pattern: %s\n", pattern ? pattern : "");

    freeStringMatchResult(&naiveRes);
    freeStringMatchResult(&rkRes);
    freeStringMatchResult(&faRes);
    freeStringMatchResult(&kmpRes);
    free(log);
}

typedef struct
{
    char module[32];
    char scenario[16];
    int inputSize;
    char algorithm[32];
    long long elapsedMicros;
    long long comparisons;
    long long swapsOrAssignments;
    long long recursiveCalls;
    long long peakExtraBytes;
    char notes[32];
} BenchmarkRow;

typedef struct
{
    BenchmarkRow* items;
    int count;
    int capacity;
} BenchmarkRowList;

static void addBenchmarkRow(
    BenchmarkRowList* list,
    const char* module,
    const char* scenario,
    int inputSize,
    const char* algorithm,
    const Metrics* m,
    const char* notes
)
{
    if (list->count >= list->capacity)
    {
        int newCap = (list->capacity == 0) ? 64 : list->capacity * 2;
        BenchmarkRow* resized = (BenchmarkRow*)realloc(list->items, (size_t)newCap * sizeof(BenchmarkRow));
        if (resized == NULL) return;
        list->items = resized;
        list->capacity = newCap;
    }

    BenchmarkRow* r = &list->items[list->count++];
    snprintf(r->module, sizeof(r->module), "%s", module);
    snprintf(r->scenario, sizeof(r->scenario), "%s", scenario);
    r->inputSize = inputSize;
    snprintf(r->algorithm, sizeof(r->algorithm), "%s", algorithm);
    r->elapsedMicros = m->elapsedMicros;
    r->comparisons = m->comparisons;
    r->swapsOrAssignments = m->swapsOrAssignments;
    r->recursiveCalls = m->recursiveCalls;
    r->peakExtraBytes = m->peakExtraBytes;
    snprintf(r->notes, sizeof(r->notes), "%s", notes);
}

static Transaction* buildSyntheticTransactions(int count, int adversarialReverse)
{
    Transaction* txs = (Transaction*)calloc((size_t)count, sizeof(Transaction));
    if (txs == NULL) return NULL;

    for (int i = 0; i < count; i++)
    {
        txs[i].transactionId = 1000 + i;
        snprintf(txs[i].senderAccount, TX_ACCOUNT_LEN, "ACC_%d", (i % 25) + 1);
        snprintf(txs[i].receiverAccount, TX_ACCOUNT_LEN, "ACC_%d", ((i * 7 + 3) % 25) + 1);
        snprintf(txs[i].deviceId, TX_ACCOUNT_LEN, "DEV_%d", (i % 10) + 1);
        snprintf(txs[i].merchantId, TX_ACCOUNT_LEN, "MERCH_%d", (i % 15) + 1);
        txs[i].amount = adversarialReverse
            ? (double)(count - i) * 10.5
            : (double)(((i * 37 + 13) % 5000) + 50);
        snprintf(txs[i].location, TX_ACCOUNT_LEN, "City_%d", i % 5);
        snprintf(txs[i].description, TX_DESC_LEN, "%s",
                 (i % 4 == 0) ? "urgent wire transfer offshore" : "regular merchant purchase");
        txs[i].isFraud = (i % 5 == 0) ? 1 : 0;
        txs[i].timestamp = 1700000000LL + (long long)i * 60LL;
    }

    return txs;
}

static void benchmarkSortingAndSearching(BenchmarkRowList* rows)
{
    const char* names[4] = {"Small", "Medium", "Large", "Adversarial"};
    const int sizes[4] = {100, 1000, 5000, 1000};

    for (int t = 0; t < 4; t++)
    {
        const char* scenario = names[t];
        int n = sizes[t];
        int isAdv = (t == 3);

        Transaction* base = buildSyntheticTransactions(n, isAdv);
        Transaction* copy = (Transaction*)malloc((size_t)n * sizeof(Transaction));
        if (!base || !copy) { free(base); free(copy); continue; }

        /* Merge Sort */
        memcpy(copy, base, (size_t)n * sizeof(Transaction));
        SortingStatistics st;
        Timer timer;
        timerStart(&timer);
        mergeSortByAmount(copy, n, &st);
        Metrics m = {st.comparisons, st.swaps, st.recursiveCalls, timerStop(&timer), (long long)n * (long long)sizeof(Transaction)};
        addBenchmarkRow(rows, "Sorting", scenario, n, "MergeSort", &m, isAdv ? "reverse-sorted" : "pseudo-random");

        /* Quick Sort */
        memcpy(copy, base, (size_t)n * sizeof(Transaction));
        timerStart(&timer);
        quickSortByAmount(copy, n, &st);
        Metrics mq = {st.comparisons, st.swaps, st.recursiveCalls, timerStop(&timer), st.recursiveCalls * 32LL};
        addBenchmarkRow(rows, "Sorting", scenario, n, "QuickSort", &mq, isAdv ? "reverse-sorted" : "pseudo-random");

        /* Binary Search (200x) */
        int targetId = isAdv ? -9999 : base[n / 2].transactionId;
        int found = -1;
        timerStart(&timer);
        for (int rep = 0; rep < 200; rep++) found = iterativeBinarySearchById(base, n, targetId);
        Metrics mbi = {0, 0, 0, timerStop(&timer), 0};
        addBenchmarkRow(rows, "Searching", scenario, n, "BinarySearch_Iterative", &mbi, found >= 0 ? "200x hit" : "200x miss");

        timerStart(&timer);
        for (int rep = 0; rep < 200; rep++) found = recursiveBinarySearchById(base, n, targetId);
        Metrics mbr = {0, 0, 0, timerStop(&timer), 0};
        addBenchmarkRow(rows, "Searching", scenario, n, "BinarySearch_Recursive", &mbr, found >= 0 ? "200x hit" : "200x miss");

        free(copy);
        free(base);
    }
}

static void benchmarkStringMatchingTiers(BenchmarkRowList* rows)
{
    const char* names[4] = {"Small", "Medium", "Large", "Adversarial"};
    const int lens[4] = {500, 5000, 20000, 5000};

    for (int t = 0; t < 4; t++)
    {
        int len = lens[t];
        int isAdv = (t == 3);
        char* text = (char*)malloc((size_t)len + 2);
        char pattern[64];
        if (!text) continue;

        if (isAdv)
        {
            memset(text, 'a', (size_t)len);
            text[len] = 'b';
            text[len + 1] = '\0';
            memset(pattern, 'a', 25);
            pattern[25] = 'b';
            pattern[26] = '\0';
        }
        else
        {
            int txCount = len / 20 + 1;
            Transaction* txs = buildSyntheticTransactions(txCount, 0);
            char* full = createTransactionLog(txs, txCount);
            free(txs);
            snprintf(text, (size_t)len + 1, "%s", full ? full : "");
            free(full);
            snprintf(pattern, sizeof(pattern), "transfer");
        }

        typedef StringMatchResult (*SMFunc)(const char*, const char*);
        SMFunc funcs[4] = {naiveStringMatching, rabinKarpStringMatching, finiteAutomatonMatching, kmpStringMatching};
        const char* algos[4] = {"Naive", "RabinKarp", "FiniteAutomaton", "KMP"};

        for (int a = 0; a < 4; a++)
        {
            Timer timer;
            timerStart(&timer);
            StringMatchResult res = funcs[a](text, pattern);
            Metrics m = {res.comparisons, res.hashCalculations + res.tableOperations, 0, timerStop(&timer), 0};
            char note[32];
            snprintf(note, sizeof(note), "matches=%d", res.matchCount);
            addBenchmarkRow(rows, "StringMatching", names[t], len, algos[a], &m, note);
            freeStringMatchResult(&res);
        }

        free(text);
    }
}

static void benchmarkDivideAndConquerTiers(BenchmarkRowList* rows)
{
    const char* names[4] = {"Small", "Medium", "Large", "Adversarial"};
    const int sizes[4] = {100, 1000, 5000, 1000};

    for (int t = 0; t < 4; t++)
    {
        int n = sizes[t];
        double* vals = (double*)malloc((size_t)n * sizeof(double));
        if (!vals) continue;
        for (int i = 0; i < n; i++)
        {
            vals[i] = (t == 3)
                ? -(double)((i % 50) + 1)
                : (((i % 3 == 0) ? 1.0 : -0.6) * ((i * 17 % 500) + 10));
        }

        Metrics mDC, mKad;
        maxSubarrayDivideConquer(vals, n, &mDC);
        addBenchmarkRow(rows, "DivideAndConquer", names[t], n, "MaxSubarray_DC", &mDC, "net-suspicious-window");
        maxSubarrayKadane(vals, n, &mKad);
        addBenchmarkRow(rows, "DivideAndConquer", names[t], n, "MaxSubarray_Kadane", &mKad, "linear-baseline");
        free(vals);
    }

    const int celebSizes[4] = {20, 80, 200, 200};
    for (int t = 0; t < 4; t++)
    {
        int n = celebSizes[t];
        int* knows = (int*)calloc((size_t)n * (size_t)n, sizeof(int));
        if (!knows) continue;
        int celeb = (t == 3) ? (n - 1) : (n / 2);
        for (int i = 0; i < n; i++)
        {
            if (i != celeb) knows[i * n + celeb] = 1;
        }
        Metrics mElim, mBrute;
        celebrityElimination(knows, n, &mElim);
        addBenchmarkRow(rows, "DivideAndConquer", names[t], n, "Celebrity_Elimination", &mElim, "O(n)");
        celebrityBruteForce(knows, n, &mBrute);
        addBenchmarkRow(rows, "DivideAndConquer", names[t], n, "Celebrity_BruteForce", &mBrute, "O(n^2)");
        free(knows);
    }

    const int matDims[4] = {16, 32, 64, 63};
    for (int t = 0; t < 4; t++)
    {
        int dim = matDims[t];
        Matrix a = createMatrix(dim, dim);
        Matrix b = createMatrix(dim, dim);
        for (int i = 0; i < dim * dim; i++) { a.data[i] = 1; b.data[i] = 2; }

        Metrics mC, mS;
        Matrix rC = multiplyClassical(&a, &b, &mC);
        addBenchmarkRow(rows, "DivideAndConquer", names[t], dim, "Matrix_Classical", &mC, t == 3 ? "non-pow2-pad" : "square");
        Matrix rS = multiplyStrassen(&a, &b, &mS);
        addBenchmarkRow(rows, "DivideAndConquer", names[t], dim, "Matrix_Strassen", &mS, t == 3 ? "non-pow2-pad" : "square");
        freeMatrix(&rC);
        freeMatrix(&rS);
        freeMatrix(&a);
        freeMatrix(&b);
    }
}

static void benchmarkGreedyTiers(BenchmarkRowList* rows)
{
    const char* names[4] = {"Small", "Medium", "Large", "Adversarial"};
    const int sizes[4] = {50, 250, 1000, 250};

    for (int t = 0; t < 4; t++)
    {
        int n = sizes[t];
        int isAdv = (t == 3);

        InvestigationSlot* slots = (InvestigationSlot*)malloc((size_t)n * sizeof(InvestigationSlot));
        CaseItem* items = (CaseItem*)malloc((size_t)n * sizeof(CaseItem));
        char* huffText = (char*)malloc((size_t)n * 10 + 1);
        if (!slots || !items || !huffText)
        {
            free(slots); free(items); free(huffText);
            continue;
        }

        huffText[0] = '\0';
        for (int i = 0; i < n; i++)
        {
            slots[i].caseId = i + 1;
            slots[i].start = isAdv ? i : (i * 3) % 500;
            slots[i].finish = isAdv ? (i + n) : (slots[i].start + (i % 7) + 1);

            items[i].caseId = i + 1;
            items[i].riskScore = (double)((i * 29) % 100 + 10);
            items[i].hoursRequired = (double)((i % 10) + 1);

            memcpy(huffText + i * 10, isAdv ? "aaaaaaaaaa" : "TXN_FRAUD_", 10);
        }
        huffText[n * 10] = '\0';

        Metrics mAct, mPart, mKnap, mHuff;
        ActivitySelectionResult act = selectActivities(slots, n, &mAct);
        addBenchmarkRow(rows, "Greedy", names[t], n, "ActivitySelection", &mAct, isAdv ? "all-overlapping" : "mixed");
        freeActivitySelectionResult(&act);

        IntervalPartitionResult part = partitionIntervals(slots, n, &mPart);
        addBenchmarkRow(rows, "Greedy", names[t], n, "IntervalPartitioning", &mPart, isAdv ? "all-overlapping" : "mixed");
        freeIntervalPartitionResult(&part);

        FractionalKnapsackResult knap = fractionalKnapsack(items, n, (double)n * 2.5, &mKnap);
        addBenchmarkRow(rows, "Greedy", names[t], n, "FractionalKnapsack", &mKnap, "triage-allocation");
        freeFractionalKnapsackResult(&knap);

        huffmanRoundTrip(huffText, &mHuff);
        addBenchmarkRow(rows, "Greedy", names[t], n * 10, "HuffmanRoundTrip", &mHuff, isAdv ? "skewed" : "normal");

        int vertices = (n / 5 < 10) ? 10 : n / 5;
        WeightedEdge* edges = (WeightedEdge*)malloc((size_t)(vertices + n) * sizeof(WeightedEdge));
        WeightedGraphData wg;
        initWeightedGraph(&wg);
        char accBuf[TX_ACCOUNT_LEN];
        for (int u = 0; u < vertices; u++)
        {
            snprintf(accBuf, sizeof(accBuf), "ACC_%d", u);
            getOrAddAccountIndex(&wg.accounts, accBuf);
        }
        int eCount = 0;
        for (int u = 0; u < vertices - 1; u++)
        {
            double w = (double)((u % 19) + 1);
            edges[eCount++] = (WeightedEdge){u, u + 1, w};
            addWeightedEdge(&wg, u, u + 1, w);
            addWeightedEdge(&wg, u + 1, u, w);
        }
        for (int e = 0; e < n; e++)
        {
            int u = e % vertices;
            int v = (e * 7 + 3) % vertices;
            if (u != v)
            {
                double w = (double)((e % 50) + 1);
                edges[eCount++] = (WeightedEdge){u, v, w};
                addWeightedEdge(&wg, u, v, w);
            }
        }

        Metrics mK, mP, mD;
        char note[32];
        snprintf(note, sizeof(note), "edges=%d", eCount);
        MstResult rk = kruskalMst(vertices, edges, eCount, &mK);
        addBenchmarkRow(rows, "Greedy", names[t], vertices, "Kruskal_MST", &mK, note);
        freeMstResult(&rk);

        MstResult rp = primMst(vertices, edges, eCount, &mP);
        addBenchmarkRow(rows, "Greedy", names[t], vertices, "Prim_MST", &mP, note);
        freeMstResult(&rp);

        DijkstraResult rd = dijkstra(&wg, 0, &mD);
        addBenchmarkRow(rows, "Greedy", names[t], vertices, "Dijkstra", &mD, note);
        freeDijkstraResult(&rd);

        freeWeightedGraph(&wg);
        free(edges);
        free(slots);
        free(items);
        free(huffText);
    }
}

void runScalabilityBenchmarkSuite(const char* csvOutputPath)
{
    printf("\n        # SCALABILITY & ADVERSARIAL BENCHMARK SUITE\n");

    BenchmarkRowList rows = {NULL, 0, 0};
    benchmarkSortingAndSearching(&rows);
    benchmarkStringMatchingTiers(&rows);
    benchmarkDivideAndConquerTiers(&rows);
    benchmarkGreedyTiers(&rows);

    printf("%-18s %-13s %-8s %-25s %-12s %-14s %-12s\n",
           "Module", "Scenario", "Size", "Algorithm", "Time(us)", "Comparisons", "ExtraBytes");
    printf("------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < rows.count; i++)
    {
        const BenchmarkRow* r = &rows.items[i];
        printf("%-18s %-13s %-8d %-25s %-12lld %-14lld %-12lld\n",
               r->module, r->scenario, r->inputSize, r->algorithm,
               r->elapsedMicros, r->comparisons, r->peakExtraBytes);
    }

    if (csvOutputPath != NULL)
    {
        FILE* out = fopen(csvOutputPath, "w");
        if (out != NULL)
        {
            fprintf(out, "module,scenario,input_size,algorithm,elapsed_us,comparisons,"
                         "swaps_or_assignments,recursive_calls,peak_extra_bytes,notes\n");
            for (int i = 0; i < rows.count; i++)
            {
                const BenchmarkRow* r = &rows.items[i];
                fprintf(out, "%s,%s,%d,%s,%lld,%lld,%lld,%lld,%lld,%s\n",
                        r->module, r->scenario, r->inputSize, r->algorithm,
                        r->elapsedMicros, r->comparisons, r->swapsOrAssignments,
                        r->recursiveCalls, r->peakExtraBytes, r->notes);
            }
            fclose(out);
            printf("\nExported %d benchmark rows to: %s\n", rows.count, csvOutputPath);
        }
    }

    free(rows.items);
}

void runCompleteBenchmark(const Transaction* transactions, int count)
{
    printf("\n============================================================\n");
    printf("       COMPARATIVE BENCHMARKING AND PROFILING ORACLE\n");
    printf("============================================================\n");

    runSortingBenchmark(transactions, count);
    runStringMatchingBenchmark(transactions, count, "transfer");
    runScalabilityBenchmarkSuite("data/benchmark_results.csv");

    printf("\nBenchmark suite completed.\n");
}
