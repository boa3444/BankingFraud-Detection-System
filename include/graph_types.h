#ifndef GRAPH_TYPES_H
#define GRAPH_TYPES_H

#include "transaction.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int from;
    int to;
    double weight;
} WeightedEdge;

typedef struct
{
    char (*names)[TX_ACCOUNT_LEN];
    int count;
    int capacity;
} AccountIndex;

typedef struct
{
    AccountIndex accounts;
    WeightedEdge* directedEdges;
    int edgeCount;
    int edgeCapacity;
} WeightedGraphData;

static inline void initAccountIndex(AccountIndex* idx)
{
    if (idx == NULL) return;
    idx->names = NULL;
    idx->count = 0;
    idx->capacity = 0;
}

static inline void freeAccountIndex(AccountIndex* idx)
{
    if (idx == NULL) return;
    free(idx->names);
    idx->names = NULL;
    idx->count = 0;
    idx->capacity = 0;
}

static inline int findAccountIndex(const AccountIndex* idx, const char* accountName)
{
    if (idx == NULL || accountName == NULL) return -1;
    for (int i = 0; i < idx->count; i++)
    {
        if (strcmp(idx->names[i], accountName) == 0)
        {
            return i;
        }
    }
    return -1;
}

static inline int getOrAddAccountIndex(AccountIndex* idx, const char* accountName)
{
    if (idx == NULL || accountName == NULL) return -1;
    int existing = findAccountIndex(idx, accountName);
    if (existing != -1) return existing;

    if (idx->count >= idx->capacity)
    {
        int newCap = (idx->capacity == 0) ? 16 : idx->capacity * 2;
        char (*resized)[TX_ACCOUNT_LEN] = (char (*)[TX_ACCOUNT_LEN])realloc(
            idx->names,
            (size_t)newCap * sizeof(*resized)
        );
        if (resized == NULL) return -1;
        idx->names = resized;
        idx->capacity = newCap;
    }

    int newIdx = idx->count++;
    snprintf(idx->names[newIdx], TX_ACCOUNT_LEN, "%s", accountName);
    return newIdx;
}

static inline void initWeightedGraph(WeightedGraphData* g)
{
    if (g == NULL) return;
    initAccountIndex(&g->accounts);
    g->directedEdges = NULL;
    g->edgeCount = 0;
    g->edgeCapacity = 0;
}

static inline void freeWeightedGraph(WeightedGraphData* g)
{
    if (g == NULL) return;
    freeAccountIndex(&g->accounts);
    free(g->directedEdges);
    g->directedEdges = NULL;
    g->edgeCount = 0;
    g->edgeCapacity = 0;
}

static inline void addWeightedEdge(WeightedGraphData* g, int from, int to, double weight)
{
    if (g == NULL) return;
    if (g->edgeCount >= g->edgeCapacity)
    {
        int newCap = (g->edgeCapacity == 0) ? 16 : g->edgeCapacity * 2;
        WeightedEdge* resized = (WeightedEdge*)realloc(
            g->directedEdges,
            (size_t)newCap * sizeof(WeightedEdge)
        );
        if (resized == NULL) return;
        g->directedEdges = resized;
        g->edgeCapacity = newCap;
    }
    g->directedEdges[g->edgeCount].from = from;
    g->directedEdges[g->edgeCount].to = to;
    g->directedEdges[g->edgeCount].weight = weight;
    g->edgeCount++;
}

static inline double fundFlowCost(double amount)
{
    if (amount < 0.0) return INFINITY;
    return 1.0 + 1.0 / (1.0 + amount / 1000.0);
}

static inline WeightedGraphData buildFundFlowGraph(
    const Transaction* transactions,
    int count
)
{
    WeightedGraphData g;
    initWeightedGraph(&g);
    if (transactions == NULL || count <= 0) return g;

    for (int i = 0; i < count; i++)
    {
        int u = getOrAddAccountIndex(&g.accounts, transactions[i].senderAccount);
        int v = getOrAddAccountIndex(&g.accounts, transactions[i].receiverAccount);
        if (u != -1 && v != -1)
        {
            addWeightedEdge(&g, u, v, fundFlowCost(transactions[i].amount));
        }
    }
    return g;
}

static inline WeightedEdge* buildLinkedAccountEdges(
    const Transaction* transactions,
    int count,
    AccountIndex* accountsOut,
    int* outEdgeCount
)
{
    initAccountIndex(accountsOut);
    if (outEdgeCount) *outEdgeCount = 0;
    if (transactions == NULL || count <= 0) return NULL;

    for (int i = 0; i < count; i++)
    {
        getOrAddAccountIndex(accountsOut, transactions[i].senderAccount);
        getOrAddAccountIndex(accountsOut, transactions[i].receiverAccount);
    }

    int edgeCap = count * 2;
    WeightedEdge* edges = (WeightedEdge*)malloc((size_t)edgeCap * sizeof(WeightedEdge));
    if (edges == NULL) return NULL;
    int edgeCount = 0;

    for (int i = 0; i < count; i++)
    {
        int u = findAccountIndex(accountsOut, transactions[i].senderAccount);
        int v = findAccountIndex(accountsOut, transactions[i].receiverAccount);
        if (u != -1 && v != -1 && u != v)
        {
            int a = (u < v) ? u : v;
            int b = (u < v) ? v : u;
            double w = fundFlowCost(transactions[i].amount);
            int found = 0;
            for (int e = 0; e < edgeCount; e++)
            {
                if (edges[e].from == a && edges[e].to == b)
                {
                    if (w < edges[e].weight) edges[e].weight = w;
                    found = 1;
                    break;
                }
            }
            if (!found && edgeCount < edgeCap)
            {
                edges[edgeCount++] = (WeightedEdge){a, b, w};
            }
        }
    }

    if (outEdgeCount) *outEdgeCount = edgeCount;
    return edges;
}

#endif
