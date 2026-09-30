#include "VitaStats.h"

#include "VitaLog.h"

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>

#include <cstdint>
#include <cstdio>

// 5 s, 30 s, then every minute
static const uint64_t REPORT_INTERVALS_US[] = { 5000000ull, 25000000ull, 60000000ull };

// A present that took longer than this missed at least one 60 Hz refresh
static constexpr uint64_t MISSED_REFRESH_US = 17500ull;

static const char* const SECTION_NAMES[VitaStats::SECTION_COUNT] =
{
    "events", "ticks", "movie", "upload", "draw", "present"
};

static uint64_t SectionStartUs[VitaStats::SECTION_COUNT] = {};
static uint64_t SectionTotalUs[VitaStats::SECTION_COUNT] = {};
static uint64_t SectionCalls[VitaStats::SECTION_COUNT] = {};

static uint64_t PeriodStartUs = 0;
static uint64_t PeriodStartThreadUs = 0;
static uint64_t LastPresentUs = 0;
static uint64_t WorstPresentIntervalUs = 0;
static uint64_t MissedRefreshes = 0;
static uint64_t Presents = 0;
static int Reports = 0;

static uint64_t NowUs()
{
    return sceKernelGetProcessTimeWide();
}

static uint64_t MainThreadCpuUs()
{
    SceKernelThreadInfo info;
    info.size = sizeof(info);
    if (sceKernelGetThreadInfo(sceKernelGetThreadId(), &info) < 0)
    {
        return 0;
    }
    return info.runClocks;
}

static void ResetPeriod(uint64_t now)
{
    for (int i = 0; i < VitaStats::SECTION_COUNT; ++i)
    {
        SectionTotalUs[i] = 0;
        SectionCalls[i] = 0;
    }
    PeriodStartUs = now;
    PeriodStartThreadUs = MainThreadCpuUs();
    WorstPresentIntervalUs = 0;
    MissedRefreshes = 0;
    Presents = 0;
}

namespace VitaStats
{
    void Begin(Section section)
    {
        SectionStartUs[section] = NowUs();
    }

    void End(Section section)
    {
        SectionTotalUs[section] += NowUs() - SectionStartUs[section];
        SectionCalls[section]++;
    }

    void FramePresented()
    {
        const uint64_t now = NowUs();
        if (PeriodStartUs == 0)
        {
            ResetPeriod(now);
            LastPresentUs = now;
            return;
        }

        const uint64_t interval = now - LastPresentUs;
        LastPresentUs = now;
        if (interval > WorstPresentIntervalUs)
        {
            WorstPresentIntervalUs = interval;
        }
        if (interval > MISSED_REFRESH_US)
        {
            MissedRefreshes++;
        }
        Presents++;

        const int scheduleIndex = (Reports < 2) ? Reports : 2;
        const uint64_t elapsed = now - PeriodStartUs;
        if (elapsed < REPORT_INTERVALS_US[scheduleIndex])
        {
            return;
        }

        const double seconds = elapsed / 1e6;
        const double presents = Presents > 0 ? static_cast<double>(Presents) : 1.0;
        const uint64_t threadUs = MainThreadCpuUs() - PeriodStartThreadUs;

        VitaLog::Write("Perf over %.1f s: %.1f game ticks/s, %.1f presents/s, %llu presents missed a refresh, worst gap %.1f ms, main thread CPU %.0f%%",
                       seconds, SectionCalls[SECTION_TICKS] / seconds, Presents / seconds,
                       static_cast<unsigned long long>(MissedRefreshes), WorstPresentIntervalUs / 1000.0,
                       100.0 * threadUs / elapsed);

        // Average milliseconds spent in each part, per presented frame
        uint64_t accountedUs = 0;
        char line[256];
        int length = 0;
        for (int i = 0; i < SECTION_COUNT; ++i)
        {
            if (i != SECTION_MOVIE)
            {
                accountedUs += SectionTotalUs[i];
            }
            length += snprintf(line + length, sizeof(line) - length, "%s%s %.2f",
                               (i == 0) ? "" : ", ", SECTION_NAMES[i], SectionTotalUs[i] / 1000.0 / presents);
        }
        const uint64_t otherUs = (elapsed > accountedUs) ? elapsed - accountedUs : 0;
        snprintf(line + length, sizeof(line) - length, ", other/sleep %.2f", otherUs / 1000.0 / presents);
        VitaLog::Write("  ms per present: %s (movie is part of ticks, %.2f ticks per present)",
                       line, SectionCalls[SECTION_TICKS] / presents);

        Reports++;
        ResetPeriod(now);
    }
}
