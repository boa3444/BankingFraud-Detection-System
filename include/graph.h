#ifndef GRAPH_H
#define GRAPH_H

#include "transaction.h"

#include <stddef.h>

#define GRAPH_NODE_NAME_LEN 96

/*
    Stores the result of one detected money-laundering ring.
*/
typedef struct
{
    char startingAccount[TX_ACCOUNT_LEN];
    char (*path)[GRAPH_NODE_NAME_LEN];
    int pathLength;
    double totalAmount;
    int ringFound;
} LaunderingRing;

/*
    Dynamic list of detected LaunderingRing results.
*/
typedef struct
{
    LaunderingRing* items;
    int count;
    int capacity;
} LaunderingRingList;

/*
    Adjacency list for a single graph node.
*/
typedef struct
{
    int* neighbors;
    int count;
    int capacity;
} GraphNodeEdges;

/*
    Directed fraud investigation graph:
    sender account -> device -> merchant -> receiver account.
*/
typedef struct
{
    char (*nodeNames)[GRAPH_NODE_NAME_LEN];
    GraphNodeEdges* adjacencyList;
    int nodeCount;
    int nodeCapacity;
} FraudGraph;

/*
    Initializes an empty FraudGraph.
*/
void initFraudGraph(FraudGraph* graph);

/*
    Frees all memory held by a FraudGraph and resets it.
*/
void clearFraudGraph(FraudGraph* graph);

/*
    Builds the directed graph from transaction records.
*/
void buildFraudGraph(
    FraudGraph* graph,
    const Transaction* transactions,
    int count
);

/*
    Displays every graph connection.
*/
void displayFraudGraph(const FraudGraph* graph);

/*
    Detects a laundering ring beginning from one account.
*/
LaunderingRing detectRingFromAccount(
    FraudGraph* graph,
    const char* accountId,
    const Transaction* transactions,
    int count
);

/*
    Detects rings beginning from all unique sender accounts.
*/
LaunderingRingList detectAllRings(
    FraudGraph* graph,
    const Transaction* transactions,
    int count
);

/*
    Frees heap memory owned by a LaunderingRing.
*/
void freeLaunderingRing(LaunderingRing* ring);

/*
    Frees heap memory owned by a LaunderingRingList.
*/
void freeLaunderingRingList(LaunderingRingList* list);

/*
    Node-naming helpers (e.g., "ACCOUNT:A101").
*/
void makeAccountNode(const char* accountId, char* out, size_t outSize);
void makeDeviceNode(const char* deviceId, char* out, size_t outSize);
void makeMerchantNode(const char* merchantId, char* out, size_t outSize);

#endif
