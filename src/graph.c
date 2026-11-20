#include "../include/graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void makeAccountNode(const char* accountId, char* out, size_t outSize)
{
    snprintf(out, outSize, "ACCOUNT:%s", accountId ? accountId : "");
}

void makeDeviceNode(const char* deviceId, char* out, size_t outSize)
{
    snprintf(out, outSize, "DEVICE:%s", deviceId ? deviceId : "");
}

void makeMerchantNode(const char* merchantId, char* out, size_t outSize)
{
    snprintf(out, outSize, "MERCHANT:%s", merchantId ? merchantId : "");
}

void initFraudGraph(FraudGraph* graph)
{
    if (graph == NULL)
    {
        return;
    }
    graph->nodeNames = NULL;
    graph->adjacencyList = NULL;
    graph->nodeCount = 0;
    graph->nodeCapacity = 0;
}

void clearFraudGraph(FraudGraph* graph)
{
    if (graph == NULL)
    {
        return;
    }

    for (int i = 0; i < graph->nodeCount; i++)
    {
        free(graph->adjacencyList[i].neighbors);
    }
    free(graph->adjacencyList);
    free(graph->nodeNames);

    graph->nodeNames = NULL;
    graph->adjacencyList = NULL;
    graph->nodeCount = 0;
    graph->nodeCapacity = 0;
}

static int findNodeIndex(const FraudGraph* graph, const char* nodeName)
{
    for (int i = 0; i < graph->nodeCount; i++)
    {
        if (strcmp(graph->nodeNames[i], nodeName) == 0)
        {
            return i;
        }
    }
    return -1;
}

static int addNode(FraudGraph* graph, const char* nodeName)
{
    int existing = findNodeIndex(graph, nodeName);
    if (existing != -1)
    {
        return existing;
    }

    if (graph->nodeCount >= graph->nodeCapacity)
    {
        int newCap = (graph->nodeCapacity == 0) ? 16 : graph->nodeCapacity * 2;
        char (*newNames)[GRAPH_NODE_NAME_LEN] = (char (*)[GRAPH_NODE_NAME_LEN])realloc(
            graph->nodeNames,
            (size_t)newCap * sizeof(*newNames)
        );
        if (newNames == NULL)
        {
            return -1;
        }
        graph->nodeNames = newNames;

        GraphNodeEdges* newAdj = (GraphNodeEdges*)realloc(
            graph->adjacencyList,
            (size_t)newCap * sizeof(GraphNodeEdges)
        );
        if (newAdj == NULL)
        {
            return -1;
        }
        graph->adjacencyList = newAdj;
        graph->nodeCapacity = newCap;
    }

    int idx = graph->nodeCount++;
    snprintf(graph->nodeNames[idx], GRAPH_NODE_NAME_LEN, "%s", nodeName);
    graph->adjacencyList[idx].neighbors = NULL;
    graph->adjacencyList[idx].count = 0;
    graph->adjacencyList[idx].capacity = 0;
    return idx;
}

static void addEdge(FraudGraph* graph, const char* fromNode, const char* toNode)
{
    int fromIdx = addNode(graph, fromNode);
    int toIdx = addNode(graph, toNode);
    if (fromIdx == -1 || toIdx == -1)
    {
        return;
    }

    GraphNodeEdges* edges = &graph->adjacencyList[fromIdx];
    for (int i = 0; i < edges->count; i++)
    {
        if (edges->neighbors[i] == toIdx)
        {
            return;
        }
    }

    if (edges->count >= edges->capacity)
    {
        int newCap = (edges->capacity == 0) ? 4 : edges->capacity * 2;
        int* resized = (int*)realloc(edges->neighbors, (size_t)newCap * sizeof(int));
        if (resized == NULL)
        {
            return;
        }
        edges->neighbors = resized;
        edges->capacity = newCap;
    }

    edges->neighbors[edges->count++] = toIdx;
}

void buildFraudGraph(
    FraudGraph* graph,
    const Transaction* transactions,
    int count
)
{
    if (graph == NULL)
    {
        return;
    }
    clearFraudGraph(graph);

    if (transactions == NULL || count <= 0)
    {
        return;
    }

    char senderNode[GRAPH_NODE_NAME_LEN];
    char deviceNode[GRAPH_NODE_NAME_LEN];
    char merchantNode[GRAPH_NODE_NAME_LEN];
    char receiverNode[GRAPH_NODE_NAME_LEN];

    for (int i = 0; i < count; i++)
    {
        makeAccountNode(transactions[i].senderAccount, senderNode, sizeof(senderNode));
        makeDeviceNode(transactions[i].deviceId, deviceNode, sizeof(deviceNode));
        makeMerchantNode(transactions[i].merchantId, merchantNode, sizeof(merchantNode));
        makeAccountNode(transactions[i].receiverAccount, receiverNode, sizeof(receiverNode));

        addEdge(graph, senderNode, deviceNode);
        addEdge(graph, deviceNode, merchantNode);
        addEdge(graph, merchantNode, receiverNode);
    }
}

void displayFraudGraph(const FraudGraph* graph)
{
    if (graph == NULL)
    {
        return;
    }

    printf("\n--------------- FRAUD INVESTIGATION GRAPH ---------------\n");
    for (int i = 0; i < graph->nodeCount; i++)
    {
        printf("%s -> ", graph->nodeNames[i]);
        const GraphNodeEdges* edges = &graph->adjacencyList[i];
        if (edges->count == 0)
        {
            printf("(none)");
        }
        else
        {
            for (int j = 0; j < edges->count; j++)
            {
                printf("%s%s",
                       graph->nodeNames[edges->neighbors[j]],
                       (j < edges->count - 1) ? ", " : "");
            }
        }
        printf("\n");
    }
}

static int recursiveDFS(
    const FraudGraph* graph,
    int currentIdx,
    int startIdx,
    int* visited,
    int* currentPath,
    int currentLen,
    int* detectedPath,
    int* detectedLen
)
{
    visited[currentIdx] = 1;
    currentPath[currentLen++] = currentIdx;

    const GraphNodeEdges* edges = &graph->adjacencyList[currentIdx];
    for (int i = 0; i < edges->count; i++)
    {
        int nextIdx = edges->neighbors[i];
        if (nextIdx == startIdx && currentLen > 1)
        {
            for (int k = 0; k < currentLen; k++)
            {
                detectedPath[k] = currentPath[k];
            }
            detectedPath[currentLen] = startIdx;
            *detectedLen = currentLen + 1;
            return 1;
        }

        if (!visited[nextIdx])
        {
            if (recursiveDFS(graph, nextIdx, startIdx, visited, currentPath, currentLen, detectedPath, detectedLen))
            {
                return 1;
            }
        }
    }

    return 0;
}

LaunderingRing detectRingFromAccount(
    FraudGraph* graph,
    const char* accountId,
    const Transaction* transactions,
    int count
)
{
    LaunderingRing result;
    memset(&result, 0, sizeof(result));
    if (accountId != NULL)
    {
        snprintf(result.startingAccount, sizeof(result.startingAccount), "%s", accountId);
    }

    if (graph == NULL || accountId == NULL || graph->nodeCount == 0)
    {
        return result;
    }

    char startNode[GRAPH_NODE_NAME_LEN];
    makeAccountNode(accountId, startNode, sizeof(startNode));
    int startIdx = findNodeIndex(graph, startNode);
    if (startIdx == -1)
    {
        return result;
    }

    int* visited = (int*)calloc((size_t)graph->nodeCount, sizeof(int));
    int* currentPath = (int*)malloc((size_t)(graph->nodeCount + 1) * sizeof(int));
    int* detectedPath = (int*)malloc((size_t)(graph->nodeCount + 1) * sizeof(int));
    int detectedLen = 0;

    if (visited != NULL && currentPath != NULL && detectedPath != NULL)
    {
        if (recursiveDFS(graph, startIdx, startIdx, visited, currentPath, 0, detectedPath, &detectedLen))
        {
            result.ringFound = 1;
            result.pathLength = detectedLen;
            result.path = (char (*)[GRAPH_NODE_NAME_LEN])malloc((size_t)detectedLen * sizeof(*result.path));
            if (result.path != NULL)
            {
                for (int i = 0; i < detectedLen; i++)
                {
                    snprintf(result.path[i], GRAPH_NODE_NAME_LEN, "%s", graph->nodeNames[detectedPath[i]]);
                }
            }

            if (transactions != NULL)
            {
                char senderNode[GRAPH_NODE_NAME_LEN];
                for (int i = 0; i < count; i++)
                {
                    makeAccountNode(transactions[i].senderAccount, senderNode, sizeof(senderNode));
                    for (int j = 0; j < result.pathLength - 1; j++)
                    {
                        if (strcmp(result.path[j], senderNode) == 0)
                        {
                            result.totalAmount += transactions[i].amount;
                            break;
                        }
                    }
                }
            }
        }
    }

    free(visited);
    free(currentPath);
    free(detectedPath);
    return result;
}

LaunderingRingList detectAllRings(
    FraudGraph* graph,
    const Transaction* transactions,
    int count
)
{
    LaunderingRingList list = {NULL, 0, 0};
    if (graph == NULL || transactions == NULL || count <= 0)
    {
        return list;
    }

    for (int i = 0; i < count; i++)
    {
        const char* acc = transactions[i].senderAccount;
        int alreadyChecked = 0;
        for (int j = 0; j < i; j++)
        {
            if (strcmp(transactions[j].senderAccount, acc) == 0)
            {
                alreadyChecked = 1;
                break;
            }
        }
        if (alreadyChecked)
        {
            continue;
        }

        LaunderingRing ring = detectRingFromAccount(graph, acc, transactions, count);
        if (ring.ringFound)
        {
            if (list.count >= list.capacity)
            {
                int newCap = (list.capacity == 0) ? 8 : list.capacity * 2;
                LaunderingRing* resized = (LaunderingRing*)realloc(
                    list.items,
                    (size_t)newCap * sizeof(LaunderingRing)
                );
                if (resized == NULL)
                {
                    freeLaunderingRing(&ring);
                    break;
                }
                list.items = resized;
                list.capacity = newCap;
            }
            list.items[list.count++] = ring;
        }
        else
        {
            freeLaunderingRing(&ring);
        }
    }

    return list;
}

void freeLaunderingRing(LaunderingRing* ring)
{
    if (ring == NULL)
    {
        return;
    }
    free(ring->path);
    ring->path = NULL;
    ring->pathLength = 0;
    ring->ringFound = 0;
}

void freeLaunderingRingList(LaunderingRingList* list)
{
    if (list == NULL)
    {
        return;
    }
    for (int i = 0; i < list->count; i++)
    {
        freeLaunderingRing(&list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}
