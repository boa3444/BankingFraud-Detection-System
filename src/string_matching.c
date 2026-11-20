#include "../include/string_matching.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void addMatch(StringMatchResult* res, int position)
{
    if (res->matchCount >= res->matchCapacity)
    {
        int newCap = (res->matchCapacity == 0) ? 16 : res->matchCapacity * 2;
        int* resized = (int*)realloc(
            res->matchingPositions,
            (size_t)newCap * sizeof(int)
        );
        if (resized == NULL)
        {
            return;
        }
        res->matchingPositions = resized;
        res->matchCapacity = newCap;
    }

    res->matchingPositions[res->matchCount++] = position;
}

static StringMatchResult emptyResult(void)
{
    StringMatchResult r;
    r.matchingPositions = NULL;
    r.matchCount = 0;
    r.matchCapacity = 0;
    r.comparisons = 0;
    r.hashCalculations = 0;
    r.tableOperations = 0;
    return r;
}

StringMatchResult naiveStringMatching(
    const char* text,
    const char* pattern
)
{
    StringMatchResult res = emptyResult();
    if (text == NULL || pattern == NULL)
    {
        return res;
    }

    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0 || m > n)
    {
        return res;
    }

    for (int i = 0; i <= n - m; i++)
    {
        int j = 0;
        while (j < m)
        {
            res.comparisons++;
            if (text[i + j] != pattern[j])
            {
                break;
            }
            j++;
        }

        if (j == m)
        {
            addMatch(&res, i);
        }
    }

    return res;
}

StringMatchResult rabinKarpStringMatching(
    const char* text,
    const char* pattern
)
{
    StringMatchResult res = emptyResult();
    if (text == NULL || pattern == NULL)
    {
        return res;
    }

    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0 || m > n)
    {
        return res;
    }

    const long long base = 256;
    const long long mod = 1000000007LL;

    long long h = 1;
    for (int i = 0; i < m - 1; i++)
    {
        h = (h * base) % mod;
        res.hashCalculations++;
    }

    long long patternHash = 0;
    long long windowHash = 0;
    for (int i = 0; i < m; i++)
    {
        patternHash = (base * patternHash + (unsigned char)pattern[i]) % mod;
        windowHash = (base * windowHash + (unsigned char)text[i]) % mod;
        res.hashCalculations += 2;
    }

    for (int i = 0; i <= n - m; i++)
    {
        if (patternHash == windowHash)
        {
            int j = 0;
            while (j < m)
            {
                res.comparisons++;
                if (text[i + j] != pattern[j])
                {
                    break;
                }
                j++;
            }
            if (j == m)
            {
                addMatch(&res, i);
            }
        }

        if (i < n - m)
        {
            long long removed = (windowHash - ((unsigned char)text[i] * h) % mod + mod) % mod;
            windowHash = (removed * base + (unsigned char)text[i + m]) % mod;
            res.hashCalculations++;
        }
    }

    return res;
}

static int computeNextState(
    const char* pattern,
    int m,
    int state,
    int ch,
    long long* tableOps
)
{
    if (state < m && (unsigned char)pattern[state] == ch)
    {
        (*tableOps)++;
        return state + 1;
    }

    for (int nextState = state; nextState > 0; nextState--)
    {
        (*tableOps)++;
        if ((unsigned char)pattern[nextState - 1] == ch)
        {
            int match = 1;
            for (int i = 0; i < nextState - 1; i++)
            {
                (*tableOps)++;
                if (pattern[i] != pattern[state - nextState + 1 + i])
                {
                    match = 0;
                    break;
                }
            }
            if (match)
            {
                return nextState;
            }
        }
    }

    return 0;
}

StringMatchResult finiteAutomatonMatching(
    const char* text,
    const char* pattern
)
{
    StringMatchResult res = emptyResult();
    if (text == NULL || pattern == NULL)
    {
        return res;
    }

    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0 || m > n)
    {
        return res;
    }

    const int alphabet = 256;
    int* table = (int*)malloc((size_t)(m + 1) * (size_t)alphabet * sizeof(int));
    if (table == NULL)
    {
        fprintf(stderr, "Error: Out of memory in finiteAutomatonMatching.\n");
        return res;
    }

    for (int state = 0; state <= m; state++)
    {
        for (int ch = 0; ch < alphabet; ch++)
        {
            table[state * alphabet + ch] =
                computeNextState(pattern, m, state, ch, &res.tableOperations);
        }
    }

    int state = 0;
    for (int i = 0; i < n; i++)
    {
        state = table[state * alphabet + (unsigned char)text[i]];
        res.comparisons++;
        if (state == m)
        {
            addMatch(&res, i - m + 1);
        }
    }

    free(table);
    return res;
}

int* buildLPSArray(
    const char* pattern,
    int* outLength
)
{
    if (pattern == NULL)
    {
        if (outLength) *outLength = 0;
        return NULL;
    }

    int m = (int)strlen(pattern);
    if (outLength) *outLength = m;
    if (m == 0)
    {
        return NULL;
    }

    int* lps = (int*)calloc((size_t)m, sizeof(int));
    if (lps == NULL)
    {
        return NULL;
    }

    int len = 0;
    int i = 1;
    while (i < m)
    {
        if (pattern[i] == pattern[len])
        {
            len++;
            lps[i] = len;
            i++;
        }
        else if (len != 0)
        {
            len = lps[len - 1];
        }
        else
        {
            lps[i] = 0;
            i++;
        }
    }

    return lps;
}

StringMatchResult kmpStringMatching(
    const char* text,
    const char* pattern
)
{
    StringMatchResult res = emptyResult();
    if (text == NULL || pattern == NULL)
    {
        return res;
    }

    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0 || m > n)
    {
        return res;
    }

    int lpsLen = 0;
    int* lps = buildLPSArray(pattern, &lpsLen);
    if (lps == NULL)
    {
        return res;
    }
    res.tableOperations += m;

    int i = 0;
    int j = 0;
    while (i < n)
    {
        res.comparisons++;
        if (text[i] == pattern[j])
        {
            i++;
            j++;
        }

        if (j == m)
        {
            addMatch(&res, i - j);
            j = lps[j - 1];
            res.tableOperations++;
        }
        else if (i < n && text[i] != pattern[j])
        {
            if (j != 0)
            {
                j = lps[j - 1];
                res.tableOperations++;
            }
            else
            {
                i++;
            }
        }
    }

    free(lps);
    return res;
}

void displayStringMatchResult(
    const char* algorithmName,
    const StringMatchResult* result
)
{
    if (result == NULL)
    {
        return;
    }

    printf("\nAlgorithm: %s\n", algorithmName ? algorithmName : "Unknown");
    printf("Matches: %d\n", result->matchCount);
    printf("Comparisons: %lld\n", result->comparisons);
    printf("Hash calculations: %lld\n", result->hashCalculations);
    printf("Table operations: %lld\n", result->tableOperations);
    if (result->matchCount > 0)
    {
        printf("Positions: ");
        for (int i = 0; i < result->matchCount; i++)
        {
            printf("%d%s", result->matchingPositions[i],
                   (i < result->matchCount - 1) ? ", " : "\n");
        }
    }
}

void freeStringMatchResult(StringMatchResult* result)
{
    if (result == NULL)
    {
        return;
    }
    free(result->matchingPositions);
    result->matchingPositions = NULL;
    result->matchCount = 0;
    result->matchCapacity = 0;
}
