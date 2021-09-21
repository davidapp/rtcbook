#include "DTimer.h"
#include <stdlib.h>


#define PERF_COUNTS 30
static DCStr	perf_desc[PERF_COUNTS] = { 0 };
static DUInt64	perf_time[PERF_COUNTS] = { 0 };
DUInt64 DTimer::overhead = 0;
DUInt64 DTimer::cpuspeed100 = 0;

DTimer::DTimer()
{
#if defined(BUILD_FOR_WINDOWS)
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    DTimer::cpuspeed100 = (DUInt32)(f.QuadPart / 10000);
#else
    DTimer::cpuspeed100 = 0;
#endif
    DTimer::overhead = 0;
}

DVoid DTimer::Start(DInt32 index, DCStr desc)
{
    perf_desc[index] = desc;
    perf_time[index] = GetCycleCount();
}

DUInt64 DTimer::Stop(DInt32 index)
{
    perf_time[index] = GetCycleCount() - perf_time[index] - DTimer::overhead;
    return perf_time[index];
}

DUInt64 DTimer::GetRes(DInt32 index)
{
    return perf_time[index];
}

DUInt64 DTimer::ClockToNSecond(DUInt64 t)
{
#if defined(BUILD_FOR_WINDOWS)
    return (t * 100000 / DTimer::cpuspeed100);
#else
    return t;
#endif
}

DUInt64 DTimer::ClockToUSecond(DUInt64 t)
{
#if defined(BUILD_FOR_WINDOWS)
    return (t * 100 / DTimer::cpuspeed100);
#else
    return (t / 1000);
#endif
}

DUInt64 DTimer::ClockToMSecond(DUInt64 t)
{
    return (t / DTimer::cpuspeed100 / 10);
}

DVoid DTimer::Output(DInt32 index, DTimeUnit unit)
{
    DStringA str = GetOutputString(index, unit);
    str.Print();
}

DStringA DTimer::GetOutputString(DInt32 index, DTimeUnit unit)
{
    DStringA strRet;
    DInt64 c = 0;
    if (unit == DTIME_IN_CLOCK)
    {
        if (perf_desc[index])
        {
            strRet.Format("%s\tUsing %llu clocks\n", perf_desc[index], perf_time[index]);
        }
        else
        {
            strRet.Format("Slot %d\tUsing %llu clocks\n", index, perf_time[index]);
        }
    }
    else if (unit == DTIME_IN_NS)
    {
        if (perf_desc[index])
        {
            strRet.Format("%s\tUsing %llu ns\n", perf_desc[index], ClockToNSecond(perf_time[index]));
        }
        else
        {
            strRet.Format("Slot %d\tUsing %llu ns\n", index, ClockToNSecond(perf_time[index]));
        }
    }
    else if (unit == DTIME_IN_US)
    {
        if (perf_desc[index])
        {
            strRet.Format("%s\tUsing %llu us\n", perf_desc[index], ClockToUSecond(perf_time[index]));
        }
        else
        {
            strRet.Format("Slot %d\tUsing %llu us\n", index, ClockToUSecond(perf_time[index]));
        }
    }
    else if (unit == DTIME_IN_MS)
    {
        c = ClockToUSecond(perf_time[index]);
        if (perf_desc[index])
        {
            strRet.Format("%s\tUsing %lld.%03lld ms\n", perf_desc[index], c / 1000, c % 1000);
        }
        else
        {
            strRet.Format("Slot %d\tUsing %lld.%03lld ms\n", index, c / 1000, c % 1000);
        }
    }
    else if (unit == DTIME_IN_SEC)
    {
        c = ClockToUSecond(perf_time[index]);
        if (perf_desc[index])
        {
            strRet.Format("%s\tUsing %lld.%06lld s\n", perf_desc[index], c / 1000000, c % 1000000);
        }
        else
        {
            strRet.Format("Slot %d\tUsing %lld.%06lld s\n", index, c / 1000000, c % 1000000);
        }
    }
    return strRet;
}

DStringA DTimer::GetOutputAllString(DTimeUnit unit)
{
    DStringA strRet;
    for (int i = 1; i < PERF_COUNTS; i++)
    {
        if (perf_time[i] != 0)
        {
            DStringA strTemp = GetOutputString(i, unit);
            strRet += strTemp;
        }
    }
    return strRet;
}

DVoid DTimer::OutputAll(DTimeUnit unit)
{
    for (int i = 1; i < PERF_COUNTS; i++)
    {
        if (perf_time[i] != 0)
        {
            Output(i, unit);
        }
    }
}

DUInt64 DTimer::GetCycleCount()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
#elif defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
    static mach_timebase_info_data_t sTimebaseInfo;
    uint64_t machTime = mach_absolute_time();
    if (sTimebaseInfo.denom == 0)
    {
        mach_timebase_info(&sTimebaseInfo);
    }
    DUInt64 nanosec = (machTime * sTimebaseInfo.numer) / sTimebaseInfo.denom;
    return nanosec;
#endif
}

DVoid DTimer::SleepSec(DUInt32 second)
{
#if defined(BUILD_FOR_WINDOWS)
    ::Sleep(second * 1000);
#else
    sleep(second);
#endif
}

