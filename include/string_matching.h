#ifndef STRING_MATCHING_H
#define STRING_MATCHING_H

/*
    Stores the result of a string-matching algorithm.
    Caller must free matchingPositions using freeStringMatchResult().
*/
typedef struct
{
    int* matchingPositions;
    int matchCount;
    int matchCapacity;
    long long comparisons;
    long long hashCalculations;
    long long tableOperations;
} StringMatchResult;

/*
    Naive string matching.
*/
StringMatchResult naiveStringMatching(
    const char* text,
    const char* pattern
);

/*
    Rabin-Karp string matching.
*/
StringMatchResult rabinKarpStringMatching(
    const char* text,
    const char* pattern
);

/*
    Finite automaton string matching.
*/
StringMatchResult finiteAutomatonMatching(
    const char* text,
    const char* pattern
);

/*
    Knuth-Morris-Pratt (KMP) string matching.
*/
StringMatchResult kmpStringMatching(
    const char* text,
    const char* pattern
);

/*
    Builds the LPS (Longest Prefix Suffix) array used by KMP.
    Caller must free the returned int* array with free().
*/
int* buildLPSArray(
    const char* pattern,
    int* outLength
);

/*
    Displays a string-matching result.
*/
void displayStringMatchResult(
    const char* algorithmName,
    const StringMatchResult* result
);

/*
    Frees heap memory owned by a StringMatchResult.
*/
void freeStringMatchResult(StringMatchResult* result);

#endif
