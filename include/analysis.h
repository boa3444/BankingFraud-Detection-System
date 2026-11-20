#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <stddef.h>
#include <time.h>

/*
    Stores common measurements collected by algorithms.
*/
typedef struct
{
    long long comparisons;
    long long swapsOrAssignments;
    long long recursiveCalls;
    long long elapsedMicros;
    long long peakExtraBytes;
} Metrics;

/*
    Measures elapsed time using C17 timespec_get().
*/
typedef struct
{
    struct timespec startTime;
    int timerStarted;
} Timer;

/*
    Resets every metric to zero.
*/
void resetMetrics(Metrics* metrics);

/*
    Timer functions.
*/
void timerInit(Timer* timer);
void timerStart(Timer* timer);
long long timerStop(Timer* timer);

/*
    Prints the heading and one aligned row for a metrics table.
*/
void printMetricsHeader(void);
void printMetricsRow(const char* name, const Metrics* metrics);

static inline void countComparison(Metrics* metrics)
{
    if (metrics != NULL)
    {
        metrics->comparisons++;
    }
}

static inline void countSwap(Metrics* metrics)
{
    if (metrics != NULL)
    {
        metrics->swapsOrAssignments++;
    }
}

static inline void countCall(Metrics* metrics)
{
    if (metrics != NULL)
    {
        metrics->recursiveCalls++;
    }
}

static inline void trackBytes(Metrics* metrics, size_t bytes)
{
    if (metrics != NULL)
    {
        long long byteCount = (long long)bytes;
        if (byteCount > metrics->peakExtraBytes)
        {
            metrics->peakExtraBytes = byteCount;
        }
    }
}

#endif
