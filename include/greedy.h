#ifndef GREEDY_H
#define GREEDY_H

#include "analysis.h"
#include "graph_types.h"
#include "transaction.h"

#include <stddef.h>

typedef struct
{
    int caseId;
    long long start;
    long long finish;
} InvestigationSlot;

typedef struct
{
    InvestigationSlot* selected;
    int count;
} ActivitySelectionResult;

typedef struct
{
    int roomCount;
    int* assignment;
    int count;
} IntervalPartitionResult;

typedef struct
{
    int caseId;
    double riskScore;
    double hoursRequired;
} CaseItem;

typedef struct
{
    double totalRisk;
    double hoursUsed;
    double* fractionTaken;
    int count;
} FractionalKnapsackResult;

typedef struct
{
    double* distance;
    int* parent;
    int vertexCount;
} DijkstraResult;

typedef struct
{
    int reachable;
    double cost;
    char (*accountPath)[TX_ACCOUNT_LEN];
    int pathCount;
} FundFlowPathResult;

typedef struct
{
    unsigned char ch;
    char code[256];
} HuffmanCodeEntry;

typedef struct
{
    HuffmanCodeEntry entries[256];
    int entryCount;
    char* encodedBits;
    size_t originalBits;
    size_t compressedBits;
    double compressionRatio;
} HuffmanResult;

typedef struct
{
    int* parent;
    int* rankValues;
    int size;
    int components;
} UnionFind;

typedef struct
{
    WeightedEdge* edges;
    int edgeCount;
    double totalWeight;
    int componentCount;
} MstResult;

/* Union-Find */
void ufInit(UnionFind* uf, int n);
int ufFind(UnionFind* uf, int value);
int ufUnite(UnionFind* uf, int first, int second);
void ufFree(UnionFind* uf);

/* Activity Selection & Interval Partitioning */
ActivitySelectionResult selectActivities(
    const InvestigationSlot* slots,
    int count,
    Metrics* metrics
);

IntervalPartitionResult partitionIntervals(
    const InvestigationSlot* slots,
    int count,
    Metrics* metrics
);

/* Fractional Knapsack */
FractionalKnapsackResult fractionalKnapsack(
    const CaseItem* cases,
    int count,
    double capacityHours,
    Metrics* metrics
);

/* Huffman Coding */
HuffmanResult buildHuffman(
    const char* text,
    Metrics* metrics
);

char* huffmanDecode(
    const char* bits,
    const HuffmanResult* tableSource
);

char* buildTransactionText(
    const Transaction* transactions,
    int count
);

int huffmanRoundTrip(
    const char* text,
    Metrics* metrics
);

/* Dijkstra Shortest Path */
DijkstraResult dijkstra(
    const WeightedGraphData* graph,
    int source,
    Metrics* metrics
);

int* reconstructPath(
    const DijkstraResult* result,
    int source,
    int target,
    int* outLen
);

FundFlowPathResult cheapestFundFlowPath(
    const Transaction* transactions,
    int count,
    const char* fromAccount,
    const char* toAccount,
    Metrics* metrics
);

/* Minimum Spanning Tree (Kruskal & Prim) */
MstResult kruskalMst(
    int vertexCount,
    const WeightedEdge* edges,
    int edgeCount,
    Metrics* metrics
);

MstResult primMst(
    int vertexCount,
    const WeightedEdge* edges,
    int edgeCount,
    Metrics* metrics
);

int compareMstTotals(const MstResult* first, const MstResult* second);

MstResult investigationNetwork(
    const Transaction* transactions,
    int count,
    int useKruskal,
    Metrics* metrics
);

/* Cleanup helpers */
void freeActivitySelectionResult(ActivitySelectionResult* r);
void freeIntervalPartitionResult(IntervalPartitionResult* r);
void freeFractionalKnapsackResult(FractionalKnapsackResult* r);
void freeDijkstraResult(DijkstraResult* r);
void freeFundFlowPathResult(FundFlowPathResult* r);
void freeHuffmanResult(HuffmanResult* r);
void freeMstResult(MstResult* r);

#endif
