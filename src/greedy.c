#include "../include/greedy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   UNION-FIND
   ============================================================ */

void ufInit(UnionFind* uf, int n)
{
    if (uf == NULL) return;
    uf->size = (n > 0) ? n : 0;
    uf->components = uf->size;
    if (uf->size == 0)
    {
        uf->parent = NULL;
        uf->rankValues = NULL;
        return;
    }

    uf->parent = (int*)malloc((size_t)uf->size * sizeof(int));
    uf->rankValues = (int*)calloc((size_t)uf->size, sizeof(int));
    for (int i = 0; i < uf->size; i++)
    {
        uf->parent[i] = i;
    }
}

int ufFind(UnionFind* uf, int value)
{
    if (uf == NULL || value < 0 || value >= uf->size) return -1;
    if (uf->parent[value] != value)
    {
        uf->parent[value] = ufFind(uf, uf->parent[value]);
    }
    return uf->parent[value];
}

int ufUnite(UnionFind* uf, int first, int second)
{
    int rootA = ufFind(uf, first);
    int rootB = ufFind(uf, second);
    if (rootA == -1 || rootB == -1 || rootA == rootB) return 0;

    if (uf->rankValues[rootA] < uf->rankValues[rootB])
    {
        uf->parent[rootA] = rootB;
    }
    else if (uf->rankValues[rootA] > uf->rankValues[rootB])
    {
        uf->parent[rootB] = rootA;
    }
    else
    {
        uf->parent[rootB] = rootA;
        uf->rankValues[rootA]++;
    }
    uf->components--;
    return 1;
}

void ufFree(UnionFind* uf)
{
    if (uf == NULL) return;
    free(uf->parent);
    free(uf->rankValues);
    uf->parent = NULL;
    uf->rankValues = NULL;
    uf->size = 0;
    uf->components = 0;
}

/* ============================================================
   INTERNAL MERGE SORT HELPERS (NO STDLIB QSORT)
   ============================================================ */

static void mergeSlotsByFinish(
    InvestigationSlot* arr,
    InvestigationSlot* tmp,
    int left,
    int mid,
    int right,
    Metrics* metrics
)
{
    int i = left, j = mid + 1, k = 0;
    while (i <= mid && j <= right)
    {
        countComparison(metrics);
        int takeLeft = (arr[i].finish < arr[j].finish) ||
            (arr[i].finish == arr[j].finish && arr[i].start <= arr[j].start);
        tmp[k++] = takeLeft ? arr[i++] : arr[j++];
        countSwap(metrics);
    }
    while (i <= mid) { tmp[k++] = arr[i++]; countSwap(metrics); }
    while (j <= right) { tmp[k++] = arr[j++]; countSwap(metrics); }
    for (int idx = 0; idx < k; idx++) arr[left + idx] = tmp[idx];
}

static void sortSlotsByFinish(
    InvestigationSlot* arr,
    InvestigationSlot* tmp,
    int left,
    int right,
    Metrics* metrics
)
{
    countCall(metrics);
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    sortSlotsByFinish(arr, tmp, left, mid, metrics);
    sortSlotsByFinish(arr, tmp, mid + 1, right, metrics);
    mergeSlotsByFinish(arr, tmp, left, mid, right, metrics);
}

typedef struct
{
    int originalIndex;
    InvestigationSlot slot;
} IndexedSlot;

static void mergeSlotsByStart(
    IndexedSlot* arr,
    IndexedSlot* tmp,
    int left,
    int mid,
    int right,
    Metrics* metrics
)
{
    int i = left, j = mid + 1, k = 0;
    while (i <= mid && j <= right)
    {
        countComparison(metrics);
        int takeLeft = (arr[i].slot.start < arr[j].slot.start) ||
            (arr[i].slot.start == arr[j].slot.start && arr[i].slot.finish <= arr[j].slot.finish);
        tmp[k++] = takeLeft ? arr[i++] : arr[j++];
        countSwap(metrics);
    }
    while (i <= mid) { tmp[k++] = arr[i++]; countSwap(metrics); }
    while (j <= right) { tmp[k++] = arr[j++]; countSwap(metrics); }
    for (int idx = 0; idx < k; idx++) arr[left + idx] = tmp[idx];
}

static void sortSlotsByStart(
    IndexedSlot* arr,
    IndexedSlot* tmp,
    int left,
    int right,
    Metrics* metrics
)
{
    countCall(metrics);
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    sortSlotsByStart(arr, tmp, left, mid, metrics);
    sortSlotsByStart(arr, tmp, mid + 1, right, metrics);
    mergeSlotsByStart(arr, tmp, left, mid, right, metrics);
}

/* ============================================================
   ACTIVITY SELECTION & INTERVAL PARTITIONING
   ============================================================ */

ActivitySelectionResult selectActivities(
    const InvestigationSlot* slots,
    int count,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    ActivitySelectionResult res = {NULL, 0};
    if (slots == NULL || count <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    InvestigationSlot* sorted = (InvestigationSlot*)malloc((size_t)count * sizeof(InvestigationSlot));
    InvestigationSlot* tmp = (InvestigationSlot*)malloc((size_t)count * sizeof(InvestigationSlot));
    res.selected = (InvestigationSlot*)malloc((size_t)count * sizeof(InvestigationSlot));
    if (!sorted || !tmp || !res.selected)
    {
        free(sorted); free(tmp); free(res.selected);
        res.selected = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    memcpy(sorted, slots, (size_t)count * sizeof(InvestigationSlot));
    sortSlotsByFinish(sorted, tmp, 0, count - 1, metrics);

    long long lastFinish = sorted[0].start - 1;
    for (int i = 0; i < count; i++)
    {
        if (sorted[i].finish < sorted[i].start) continue;
        countComparison(metrics);
        if (res.count == 0 || sorted[i].start >= lastFinish)
        {
            res.selected[res.count++] = sorted[i];
            lastFinish = sorted[i].finish;
            countSwap(metrics);
        }
    }

    free(sorted);
    free(tmp);
    trackBytes(metrics, (size_t)count * sizeof(InvestigationSlot) * 2);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

typedef struct
{
    long long finish;
    int room;
} RoomEnd;

static void heapPushRoom(RoomEnd* heap, int* size, RoomEnd val, Metrics* metrics)
{
    int idx = (*size)++;
    heap[idx] = val;
    while (idx > 0)
    {
        int parent = (idx - 1) / 2;
        countComparison(metrics);
        if (heap[idx].finish < heap[parent].finish)
        {
            RoomEnd t = heap[idx]; heap[idx] = heap[parent]; heap[parent] = t;
            countSwap(metrics);
            idx = parent;
        }
        else break;
    }
}

static RoomEnd heapPopRoom(RoomEnd* heap, int* size, Metrics* metrics)
{
    RoomEnd top = heap[0];
    heap[0] = heap[--(*size)];
    int idx = 0;
    while (1)
    {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;
        if (left < *size)
        {
            countComparison(metrics);
            if (heap[left].finish < heap[smallest].finish) smallest = left;
        }
        if (right < *size)
        {
            countComparison(metrics);
            if (heap[right].finish < heap[smallest].finish) smallest = right;
        }
        if (smallest != idx)
        {
            RoomEnd t = heap[idx]; heap[idx] = heap[smallest]; heap[smallest] = t;
            countSwap(metrics);
            idx = smallest;
        }
        else break;
    }
    return top;
}

IntervalPartitionResult partitionIntervals(
    const InvestigationSlot* slots,
    int count,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    IntervalPartitionResult res = {0, NULL, 0};
    if (slots == NULL || count <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    IndexedSlot* indexed = (IndexedSlot*)malloc((size_t)count * sizeof(IndexedSlot));
    IndexedSlot* tmp = (IndexedSlot*)malloc((size_t)count * sizeof(IndexedSlot));
    RoomEnd* heap = (RoomEnd*)malloc((size_t)count * sizeof(RoomEnd));
    res.assignment = (int*)calloc((size_t)count, sizeof(int));
    if (!indexed || !tmp || !heap || !res.assignment)
    {
        free(indexed); free(tmp); free(heap); free(res.assignment);
        res.assignment = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int i = 0; i < count; i++)
    {
        indexed[i].originalIndex = i;
        indexed[i].slot = slots[i];
    }
    sortSlotsByStart(indexed, tmp, 0, count - 1, metrics);

    int heapSize = 0;
    for (int i = 0; i < count; i++)
    {
        if (heapSize > 0)
        {
            countComparison(metrics);
            if (heap[0].finish <= indexed[i].slot.start)
            {
                RoomEnd reused = heapPopRoom(heap, &heapSize, metrics);
                reused.finish = indexed[i].slot.finish;
                res.assignment[indexed[i].originalIndex] = reused.room;
                heapPushRoom(heap, &heapSize, reused, metrics);
                continue;
            }
        }
        int newRoom = res.roomCount++;
        res.assignment[indexed[i].originalIndex] = newRoom;
        heapPushRoom(heap, &heapSize, (RoomEnd){indexed[i].slot.finish, newRoom}, metrics);
    }

    res.count = count;
    free(indexed);
    free(tmp);
    free(heap);
    trackBytes(metrics, (size_t)count * sizeof(IndexedSlot) * 2);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

/* ============================================================
   FRACTIONAL KNAPSACK
   ============================================================ */

typedef struct
{
    int originalIndex;
    CaseItem item;
} IndexedCase;

static void mergeCasesByRatio(
    IndexedCase* arr,
    IndexedCase* tmp,
    int left,
    int mid,
    int right,
    Metrics* metrics
)
{
    int i = left, j = mid + 1, k = 0;
    while (i <= mid && j <= right)
    {
        double r1 = arr[i].item.riskScore / arr[i].item.hoursRequired;
        double r2 = arr[j].item.riskScore / arr[j].item.hoursRequired;
        countComparison(metrics);
        tmp[k++] = (r1 >= r2) ? arr[i++] : arr[j++];
        countSwap(metrics);
    }
    while (i <= mid) { tmp[k++] = arr[i++]; countSwap(metrics); }
    while (j <= right) { tmp[k++] = arr[j++]; countSwap(metrics); }
    for (int idx = 0; idx < k; idx++) arr[left + idx] = tmp[idx];
}

static void sortCasesByRatio(
    IndexedCase* arr,
    IndexedCase* tmp,
    int left,
    int right,
    Metrics* metrics
)
{
    countCall(metrics);
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    sortCasesByRatio(arr, tmp, left, mid, metrics);
    sortCasesByRatio(arr, tmp, mid + 1, right, metrics);
    mergeCasesByRatio(arr, tmp, left, mid, right, metrics);
}

FractionalKnapsackResult fractionalKnapsack(
    const CaseItem* cases,
    int count,
    double capacityHours,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    FractionalKnapsackResult res = {0.0, 0.0, NULL, 0};
    if (cases == NULL || count <= 0 || capacityHours <= 0.0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    IndexedCase* items = (IndexedCase*)malloc((size_t)count * sizeof(IndexedCase));
    IndexedCase* tmp = (IndexedCase*)malloc((size_t)count * sizeof(IndexedCase));
    res.fractionTaken = (double*)calloc((size_t)count, sizeof(double));
    if (!items || !tmp || !res.fractionTaken)
    {
        free(items); free(tmp); free(res.fractionTaken);
        res.fractionTaken = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int validCount = 0;
    for (int i = 0; i < count; i++)
    {
        if (cases[i].hoursRequired > 0.0 && cases[i].riskScore >= 0.0)
        {
            items[validCount].originalIndex = i;
            items[validCount].item = cases[i];
            validCount++;
        }
    }

    if (validCount > 0)
    {
        sortCasesByRatio(items, tmp, 0, validCount - 1, metrics);
    }

    double remaining = capacityHours;
    for (int i = 0; i < validCount && remaining > 0.0; i++)
    {
        double take = (items[i].item.hoursRequired <= remaining)
            ? 1.0
            : (remaining / items[i].item.hoursRequired);
        res.fractionTaken[items[i].originalIndex] = take;
        res.totalRisk += take * items[i].item.riskScore;
        res.hoursUsed += take * items[i].item.hoursRequired;
        remaining -= take * items[i].item.hoursRequired;
        countSwap(metrics);
    }

    res.count = count;
    free(items);
    free(tmp);
    trackBytes(metrics, (size_t)count * sizeof(IndexedCase) * 2);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

/* ============================================================
   HUFFMAN CODING
   ============================================================ */

typedef struct HuffmanNode
{
    unsigned char ch;
    long long freq;
    int order;
    struct HuffmanNode* left;
    struct HuffmanNode* right;
} HuffmanNode;

static void heapPushHuff(HuffmanNode** heap, int* size, HuffmanNode* node, Metrics* metrics)
{
    int idx = (*size)++;
    heap[idx] = node;
    while (idx > 0)
    {
        int parent = (idx - 1) / 2;
        countComparison(metrics);
        int less = (heap[idx]->freq < heap[parent]->freq) ||
            (heap[idx]->freq == heap[parent]->freq && heap[idx]->order < heap[parent]->order);
        if (less)
        {
            HuffmanNode* t = heap[idx]; heap[idx] = heap[parent]; heap[parent] = t;
            countSwap(metrics);
            idx = parent;
        }
        else break;
    }
}

static HuffmanNode* heapPopHuff(HuffmanNode** heap, int* size, Metrics* metrics)
{
    HuffmanNode* top = heap[0];
    heap[0] = heap[--(*size)];
    int idx = 0;
    while (1)
    {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;
        if (left < *size)
        {
            countComparison(metrics);
            if (heap[left]->freq < heap[smallest]->freq ||
                (heap[left]->freq == heap[smallest]->freq && heap[left]->order < heap[smallest]->order))
                smallest = left;
        }
        if (right < *size)
        {
            countComparison(metrics);
            if (heap[right]->freq < heap[smallest]->freq ||
                (heap[right]->freq == heap[smallest]->freq && heap[right]->order < heap[smallest]->order))
                smallest = right;
        }
        if (smallest != idx)
        {
            HuffmanNode* t = heap[idx]; heap[idx] = heap[smallest]; heap[smallest] = t;
            countSwap(metrics);
            idx = smallest;
        }
        else break;
    }
    return top;
}

static void assignHuffCodes(
    const HuffmanNode* node,
    const char* prefix,
    char codes[256][256]
)
{
    if (node == NULL) return;
    if (node->left == NULL && node->right == NULL)
    {
        if (prefix[0] == '\0')
        {
            snprintf(codes[node->ch], 256, "0");
        }
        else
        {
            snprintf(codes[node->ch], 256, "%s", prefix);
        }
        return;
    }

    char leftBuf[256], rightBuf[256];
    snprintf(leftBuf, sizeof(leftBuf), "%s0", prefix);
    snprintf(rightBuf, sizeof(rightBuf), "%s1", prefix);
    assignHuffCodes(node->left, leftBuf, codes);
    assignHuffCodes(node->right, rightBuf, codes);
}

static void freeHuffTree(HuffmanNode* node)
{
    if (node == NULL) return;
    freeHuffTree(node->left);
    freeHuffTree(node->right);
    free(node);
}

HuffmanResult buildHuffman(const char* text, Metrics* metrics)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    HuffmanResult res;
    memset(&res, 0, sizeof(res));
    res.compressionRatio = 1.0;

    if (text == NULL || text[0] == '\0')
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    size_t len = strlen(text);
    res.originalBits = len * 8;

    long long freq[256] = {0};
    for (size_t i = 0; i < len; i++)
    {
        freq[(unsigned char)text[i]]++;
        countSwap(metrics);
    }

    HuffmanNode* heap[512];
    int heapSize = 0;
    int nextOrder = 0;
    for (int c = 0; c < 256; c++)
    {
        if (freq[c] > 0)
        {
            HuffmanNode* leaf = (HuffmanNode*)malloc(sizeof(HuffmanNode));
            leaf->ch = (unsigned char)c;
            leaf->freq = freq[c];
            leaf->order = nextOrder++;
            leaf->left = NULL;
            leaf->right = NULL;
            heapPushHuff(heap, &heapSize, leaf, metrics);
        }
    }

    while (heapSize > 1)
    {
        HuffmanNode* left = heapPopHuff(heap, &heapSize, metrics);
        HuffmanNode* right = heapPopHuff(heap, &heapSize, metrics);
        HuffmanNode* merged = (HuffmanNode*)malloc(sizeof(HuffmanNode));
        merged->ch = 0;
        merged->freq = left->freq + right->freq;
        merged->order = nextOrder++;
        merged->left = left;
        merged->right = right;
        heapPushHuff(heap, &heapSize, merged, metrics);
    }

    HuffmanNode* root = heapPopHuff(heap, &heapSize, metrics);
    char codes[256][256];
    memset(codes, 0, sizeof(codes));
    assignHuffCodes(root, "", codes);
    freeHuffTree(root);

    size_t totalBits = 0;
    for (int c = 0; c < 256; c++)
    {
        if (freq[c] > 0)
        {
            res.entries[res.entryCount].ch = (unsigned char)c;
            snprintf(res.entries[res.entryCount].code, 256, "%s", codes[c]);
            res.entryCount++;
            totalBits += (size_t)freq[c] * strlen(codes[c]);
        }
    }

    res.encodedBits = (char*)malloc(totalBits + 1);
    if (res.encodedBits != NULL)
    {
        size_t pos = 0;
        for (size_t i = 0; i < len; i++)
        {
            const char* code = codes[(unsigned char)text[i]];
            size_t clen = strlen(code);
            memcpy(res.encodedBits + pos, code, clen);
            pos += clen;
            countSwap(metrics);
        }
        res.encodedBits[pos] = '\0';
    }

    res.compressedBits = totalBits;
    res.compressionRatio = res.originalBits > 0
        ? (double)res.compressedBits / (double)res.originalBits
        : 1.0;

    trackBytes(metrics, totalBits + sizeof(res));
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

char* huffmanDecode(const char* bits, const HuffmanResult* tableSource)
{
    if (bits == NULL || tableSource == NULL || bits[0] == '\0')
    {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    size_t bitsLen = strlen(bits);
    char* out = (char*)malloc(bitsLen + 1);
    if (out == NULL) return NULL;
    size_t outLen = 0;

    char current[256];
    size_t curLen = 0;
    for (size_t i = 0; i < bitsLen; i++)
    {
        if (curLen < 255)
        {
            current[curLen++] = bits[i];
            current[curLen] = '\0';
        }
        for (int e = 0; e < tableSource->entryCount; e++)
        {
            if (strcmp(current, tableSource->entries[e].code) == 0)
            {
                out[outLen++] = (char)tableSource->entries[e].ch;
                curLen = 0;
                current[0] = '\0';
                break;
            }
        }
    }
    out[outLen] = '\0';
    return out;
}

char* buildTransactionText(const Transaction* transactions, int count)
{
    if (transactions == NULL || count <= 0)
    {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    size_t cap = (size_t)count * 400 + 1;
    char* buf = (char*)malloc(cap);
    if (buf == NULL) return NULL;
    size_t pos = 0;

    for (int i = 0; i < count; i++)
    {
        int written = snprintf(buf + pos, cap - pos, "%d|%s->%s|%s\n",
                               transactions[i].transactionId,
                               transactions[i].senderAccount,
                               transactions[i].receiverAccount,
                               transactions[i].description);
        if (written > 0) pos += (size_t)written;
    }
    return buf;
}

int huffmanRoundTrip(const char* text, Metrics* metrics)
{
    if (text == NULL) return 1;
    HuffmanResult enc = buildHuffman(text, metrics);
    char* dec = huffmanDecode(enc.encodedBits, &enc);
    int ok = (dec != NULL && strcmp(dec, text) == 0);
    free(dec);
    freeHuffmanResult(&enc);
    return ok;
}

/* ============================================================
   DIJKSTRA SHORTEST PATH
   ============================================================ */

typedef struct
{
    int vertex;
    double dist;
} DijkNode;

static void heapPushDijk(DijkNode* heap, int* size, DijkNode val, Metrics* metrics)
{
    int idx = (*size)++;
    heap[idx] = val;
    while (idx > 0)
    {
        int parent = (idx - 1) / 2;
        countComparison(metrics);
        if (heap[idx].dist < heap[parent].dist)
        {
            DijkNode t = heap[idx]; heap[idx] = heap[parent]; heap[parent] = t;
            countSwap(metrics);
            idx = parent;
        }
        else break;
    }
}

static DijkNode heapPopDijk(DijkNode* heap, int* size, Metrics* metrics)
{
    DijkNode top = heap[0];
    heap[0] = heap[--(*size)];
    int idx = 0;
    while (1)
    {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;
        if (left < *size)
        {
            countComparison(metrics);
            if (heap[left].dist < heap[smallest].dist) smallest = left;
        }
        if (right < *size)
        {
            countComparison(metrics);
            if (heap[right].dist < heap[smallest].dist) smallest = right;
        }
        if (smallest != idx)
        {
            DijkNode t = heap[idx]; heap[idx] = heap[smallest]; heap[smallest] = t;
            countSwap(metrics);
            idx = smallest;
        }
        else break;
    }
    return top;
}

DijkstraResult dijkstra(
    const WeightedGraphData* graph,
    int source,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    DijkstraResult res = {NULL, NULL, 0};
    if (graph == NULL || graph->accounts.count <= 0 ||
        source < 0 || source >= graph->accounts.count)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int n = graph->accounts.count;
    res.vertexCount = n;
    res.distance = (double*)malloc((size_t)n * sizeof(double));
    res.parent = (int*)malloc((size_t)n * sizeof(int));
    int* visited = (int*)calloc((size_t)n, sizeof(int));
    int heapCap = graph->edgeCount + n + 1;
    DijkNode* heap = (DijkNode*)malloc((size_t)heapCap * sizeof(DijkNode));

    if (!res.distance || !res.parent || !visited || !heap)
    {
        free(visited); free(heap);
        freeDijkstraResult(&res);
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    for (int i = 0; i < n; i++)
    {
        res.distance[i] = INFINITY;
        res.parent[i] = -1;
    }
    res.distance[source] = 0.0;

    int heapSize = 0;
    heapPushDijk(heap, &heapSize, (DijkNode){source, 0.0}, metrics);

    while (heapSize > 0)
    {
        DijkNode cur = heapPopDijk(heap, &heapSize, metrics);
        if (visited[cur.vertex]) continue;
        visited[cur.vertex] = 1;

        for (int e = 0; e < graph->edgeCount; e++)
        {
            const WeightedEdge* edge = &graph->directedEdges[e];
            if (edge->from != cur.vertex || edge->weight < 0.0) continue;
            countComparison(metrics);
            double cand = res.distance[cur.vertex] + edge->weight;
            if (cand < res.distance[edge->to])
            {
                res.distance[edge->to] = cand;
                res.parent[edge->to] = cur.vertex;
                countSwap(metrics);
                if (heapSize < heapCap)
                {
                    heapPushDijk(heap, &heapSize, (DijkNode){edge->to, cand}, metrics);
                }
            }
        }
    }

    free(visited);
    free(heap);
    trackBytes(metrics, (size_t)n * (sizeof(double) + sizeof(int)));
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

int* reconstructPath(
    const DijkstraResult* result,
    int source,
    int target,
    int* outLen
)
{
    if (outLen) *outLen = 0;
    if (result == NULL || target < 0 || target >= result->vertexCount ||
        !isfinite(result->distance[target]))
    {
        return NULL;
    }

    int* rev = (int*)malloc((size_t)result->vertexCount * sizeof(int));
    if (rev == NULL) return NULL;
    int len = 0;
    for (int cur = target; cur != -1 && len < result->vertexCount; cur = result->parent[cur])
    {
        rev[len++] = cur;
        if (cur == source) break;
    }

    if (len == 0 || rev[len - 1] != source)
    {
        free(rev);
        return NULL;
    }

    int* path = (int*)malloc((size_t)len * sizeof(int));
    for (int i = 0; i < len; i++)
    {
        path[i] = rev[len - 1 - i];
    }
    free(rev);
    if (outLen) *outLen = len;
    return path;
}

FundFlowPathResult cheapestFundFlowPath(
    const Transaction* transactions,
    int count,
    const char* fromAccount,
    const char* toAccount,
    Metrics* metrics
)
{
    FundFlowPathResult res = {0, INFINITY, NULL, 0};
    WeightedGraphData g = buildFundFlowGraph(transactions, count);
    int src = findAccountIndex(&g.accounts, fromAccount);
    int dst = findAccountIndex(&g.accounts, toAccount);

    if (src == -1 || dst == -1)
    {
        freeWeightedGraph(&g);
        return res;
    }

    DijkstraResult dr = dijkstra(&g, src, metrics);
    int pathLen = 0;
    int* pathIdx = reconstructPath(&dr, src, dst, &pathLen);
    if (pathIdx != NULL && pathLen > 0)
    {
        res.reachable = 1;
        res.cost = dr.distance[dst];
        res.pathCount = pathLen;
        res.accountPath = (char (*)[TX_ACCOUNT_LEN])malloc((size_t)pathLen * sizeof(*res.accountPath));
        for (int i = 0; i < pathLen; i++)
        {
            snprintf(res.accountPath[i], TX_ACCOUNT_LEN, "%s", g.accounts.names[pathIdx[i]]);
        }
    }

    free(pathIdx);
    freeDijkstraResult(&dr);
    freeWeightedGraph(&g);
    return res;
}

/* ============================================================
   MINIMUM SPANNING TREE (KRUSKAL & PRIM)
   ============================================================ */

static void mergeEdges(
    WeightedEdge* arr,
    WeightedEdge* tmp,
    int left,
    int mid,
    int right,
    Metrics* metrics
)
{
    int i = left, j = mid + 1, k = 0;
    while (i <= mid && j <= right)
    {
        countComparison(metrics);
        tmp[k++] = (arr[i].weight <= arr[j].weight) ? arr[i++] : arr[j++];
        countSwap(metrics);
    }
    while (i <= mid) { tmp[k++] = arr[i++]; countSwap(metrics); }
    while (j <= right) { tmp[k++] = arr[j++]; countSwap(metrics); }
    for (int idx = 0; idx < k; idx++) arr[left + idx] = tmp[idx];
}

static void sortEdges(
    WeightedEdge* arr,
    WeightedEdge* tmp,
    int left,
    int right,
    Metrics* metrics
)
{
    countCall(metrics);
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    sortEdges(arr, tmp, left, mid, metrics);
    sortEdges(arr, tmp, mid + 1, right, metrics);
    mergeEdges(arr, tmp, left, mid, right, metrics);
}

MstResult kruskalMst(
    int vertexCount,
    const WeightedEdge* edges,
    int edgeCount,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    MstResult res = {NULL, 0, 0.0, (vertexCount > 0) ? vertexCount : 0};
    if (vertexCount <= 0 || edges == NULL || edgeCount <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    WeightedEdge* valid = (WeightedEdge*)malloc((size_t)edgeCount * sizeof(WeightedEdge));
    WeightedEdge* tmp = (WeightedEdge*)malloc((size_t)edgeCount * sizeof(WeightedEdge));
    res.edges = (WeightedEdge*)malloc((size_t)vertexCount * sizeof(WeightedEdge));
    if (!valid || !tmp || !res.edges)
    {
        free(valid); free(tmp); free(res.edges);
        res.edges = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int vCount = 0;
    for (int i = 0; i < edgeCount; i++)
    {
        if (edges[i].from >= 0 && edges[i].from < vertexCount &&
            edges[i].to >= 0 && edges[i].to < vertexCount &&
            edges[i].from != edges[i].to && edges[i].weight >= 0.0)
        {
            valid[vCount++] = edges[i];
        }
    }

    if (vCount > 0)
    {
        sortEdges(valid, tmp, 0, vCount - 1, metrics);
    }

    UnionFind uf;
    ufInit(&uf, vertexCount);
    for (int i = 0; i < vCount; i++)
    {
        countComparison(metrics);
        if (ufUnite(&uf, valid[i].from, valid[i].to))
        {
            res.edges[res.edgeCount++] = valid[i];
            res.totalWeight += valid[i].weight;
            countSwap(metrics);
        }
    }

    res.componentCount = uf.components;
    ufFree(&uf);
    free(valid);
    free(tmp);
    trackBytes(metrics, (size_t)vCount * sizeof(WeightedEdge) * 2);
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

static void heapPushEdge(WeightedEdge* heap, int* size, WeightedEdge val, Metrics* metrics)
{
    int idx = (*size)++;
    heap[idx] = val;
    while (idx > 0)
    {
        int parent = (idx - 1) / 2;
        countComparison(metrics);
        if (heap[idx].weight < heap[parent].weight)
        {
            WeightedEdge t = heap[idx]; heap[idx] = heap[parent]; heap[parent] = t;
            countSwap(metrics);
            idx = parent;
        }
        else break;
    }
}

static WeightedEdge heapPopEdge(WeightedEdge* heap, int* size, Metrics* metrics)
{
    WeightedEdge top = heap[0];
    heap[0] = heap[--(*size)];
    int idx = 0;
    while (1)
    {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;
        if (left < *size)
        {
            countComparison(metrics);
            if (heap[left].weight < heap[smallest].weight) smallest = left;
        }
        if (right < *size)
        {
            countComparison(metrics);
            if (heap[right].weight < heap[smallest].weight) smallest = right;
        }
        if (smallest != idx)
        {
            WeightedEdge t = heap[idx]; heap[idx] = heap[smallest]; heap[smallest] = t;
            countSwap(metrics);
            idx = smallest;
        }
        else break;
    }
    return top;
}

MstResult primMst(
    int vertexCount,
    const WeightedEdge* edges,
    int edgeCount,
    Metrics* metrics
)
{
    resetMetrics(metrics);
    Timer timer;
    timerStart(&timer);

    MstResult res = {NULL, 0, 0.0, 0};
    if (vertexCount <= 0)
    {
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int* visited = (int*)calloc((size_t)vertexCount, sizeof(int));
    int heapCap = (edgeCount > 0 ? edgeCount * 2 : 1) + 1;
    WeightedEdge* heap = (WeightedEdge*)malloc((size_t)heapCap * sizeof(WeightedEdge));
    res.edges = (WeightedEdge*)malloc((size_t)vertexCount * sizeof(WeightedEdge));
    if (!visited || !heap || !res.edges)
    {
        free(visited); free(heap); free(res.edges);
        res.edges = NULL;
        if (metrics) metrics->elapsedMicros = timerStop(&timer);
        return res;
    }

    int heapSize = 0;
    for (int start = 0; start < vertexCount; start++)
    {
        if (visited[start]) continue;
        res.componentCount++;
        visited[start] = 1;

        for (int e = 0; e < edgeCount; e++)
        {
            if (edges[e].weight < 0.0) continue;
            if (edges[e].from == start && !visited[edges[e].to])
            {
                heapPushEdge(heap, &heapSize, (WeightedEdge){start, edges[e].to, edges[e].weight}, metrics);
            }
            else if (edges[e].to == start && !visited[edges[e].from])
            {
                heapPushEdge(heap, &heapSize, (WeightedEdge){start, edges[e].from, edges[e].weight}, metrics);
            }
        }

        while (heapSize > 0)
        {
            WeightedEdge best = heapPopEdge(heap, &heapSize, metrics);
            countComparison(metrics);
            if (visited[best.to]) continue;

            visited[best.to] = 1;
            res.edges[res.edgeCount++] = best;
            res.totalWeight += best.weight;
            countSwap(metrics);

            int u = best.to;
            for (int e = 0; e < edgeCount; e++)
            {
                if (edges[e].weight < 0.0) continue;
                if (edges[e].from == u && !visited[edges[e].to] && heapSize < heapCap)
                {
                    heapPushEdge(heap, &heapSize, (WeightedEdge){u, edges[e].to, edges[e].weight}, metrics);
                }
                else if (edges[e].to == u && !visited[edges[e].from] && heapSize < heapCap)
                {
                    heapPushEdge(heap, &heapSize, (WeightedEdge){u, edges[e].from, edges[e].weight}, metrics);
                }
            }
        }
    }

    free(visited);
    free(heap);
    trackBytes(metrics, (size_t)heapCap * sizeof(WeightedEdge));
    if (metrics) metrics->elapsedMicros = timerStop(&timer);
    return res;
}

int compareMstTotals(const MstResult* first, const MstResult* second)
{
    if (!first || !second) return 0;
    double diff = first->totalWeight - second->totalWeight;
    if (diff < 0.0) diff = -diff;
    return diff < 1e-6 && first->componentCount == second->componentCount;
}

MstResult investigationNetwork(
    const Transaction* transactions,
    int count,
    int useKruskal,
    Metrics* metrics
)
{
    AccountIndex accounts;
    int edgeCount = 0;
    WeightedEdge* edges = buildLinkedAccountEdges(transactions, count, &accounts, &edgeCount);
    int vertexCount = accounts.count;

    MstResult res = useKruskal
        ? kruskalMst(vertexCount, edges, edgeCount, metrics)
        : primMst(vertexCount, edges, edgeCount, metrics);

    free(edges);
    freeAccountIndex(&accounts);
    return res;
}

/* ============================================================
   CLEANUP HELPERS
   ============================================================ */

void freeActivitySelectionResult(ActivitySelectionResult* r)
{
    if (!r) return;
    free(r->selected);
    r->selected = NULL;
    r->count = 0;
}

void freeIntervalPartitionResult(IntervalPartitionResult* r)
{
    if (!r) return;
    free(r->assignment);
    r->assignment = NULL;
    r->count = 0;
}

void freeFractionalKnapsackResult(FractionalKnapsackResult* r)
{
    if (!r) return;
    free(r->fractionTaken);
    r->fractionTaken = NULL;
    r->count = 0;
}

void freeDijkstraResult(DijkstraResult* r)
{
    if (!r) return;
    free(r->distance);
    free(r->parent);
    r->distance = NULL;
    r->parent = NULL;
    r->vertexCount = 0;
}

void freeFundFlowPathResult(FundFlowPathResult* r)
{
    if (!r) return;
    free(r->accountPath);
    r->accountPath = NULL;
    r->pathCount = 0;
}

void freeHuffmanResult(HuffmanResult* r)
{
    if (!r) return;
    free(r->encodedBits);
    r->encodedBits = NULL;
}

void freeMstResult(MstResult* r)
{
    if (!r) return;
    free(r->edges);
    r->edges = NULL;
    r->edgeCount = 0;
}
