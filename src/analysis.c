#include "../include/analysis.h"

#include <stdio.h>

void resetMetrics(Metrics* metrics)
{
    if (metrics == NULL)
    {
        return;
    }

    metrics->comparisons = 0;
    metrics->swapsOrAssignments = 0;
    metrics->recursiveCalls = 0;
    metrics->elapsedMicros = 0;
    metrics->peakExtraBytes = 0;
}

void timerInit(Timer* timer)
{
    if (timer == NULL)
    {
        return;
    }
    timer->startTime.tv_sec = 0;
    timer->startTime.tv_nsec = 0;
    timer->timerStarted = 0;
}

void timerStart(Timer* timer)
{
    if (timer == NULL)
    {
        return;
    }
    timespec_get(&timer->startTime, TIME_UTC);
    timer->timerStarted = 1;
}

long long timerStop(Timer* timer)
{
    if (timer == NULL || !timer->timerStarted)
    {
        return 0;
    }

    struct timespec endTime;
    timespec_get(&endTime, TIME_UTC);

    long long secDiff = (long long)(endTime.tv_sec - timer->startTime.tv_sec);
    long long nsecDiff = (long long)(endTime.tv_nsec - timer->startTime.tv_nsec);
    long long totalMicros = secDiff * 1000000LL + nsecDiff / 1000LL;

    timer->timerStarted = 0;
    return (totalMicros >= 0) ? totalMicros : 0;
}

void printMetricsHeader(void)
{
    printf("%-24s %-12s %-14s %-14s %-12s %-12s\n",
           "Algorithm", "Time(us)", "Comparisons", "Swaps/Moves", "RecCalls", "ExtraBytes");
    printf("----------------------------------------------------------------------------------------\n");
}

void printMetricsRow(const char* name, const Metrics* metrics)
{
    if (metrics == NULL)
    {
        return;
    }

    printf("%-24s %-12lld %-14lld %-14lld %-12lld %-12lld\n",
           name ? name : "Unknown",
           metrics->elapsedMicros,
           metrics->comparisons,
           metrics->swapsOrAssignments,
           metrics->recursiveCalls,
           metrics->peakExtraBytes);
}
