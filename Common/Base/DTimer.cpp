#include "DTimer.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>

#define PERF_COUNTS 30
static DCStr	perf_desc[PERF_COUNTS] = { 0 };
static DUInt64	perf_time[PERF_COUNTS] = { 0 };
DUInt64 DTimer::overhead = 0;
DUInt64 DTimer::cpuspeed100 = 0;

DVoid DTimer::Init()
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
    perf_time[index] = DTimer::GetCycleCount();
}

DUInt64 DTimer::Stop(DInt32 index)
{
    if (perf_time[index] == 0) return 0;
    perf_time[index] = DTimer::GetCycleCount() - perf_time[index] - DTimer::overhead;
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
    if (perf_time[index] == 0) return;

    std::string str = GetOutputString(index, unit);
    std::cout << str << std::endl;

#if defined(BUILD_FOR_WINDOWS)
    OutputDebugStringA(str.c_str());
#endif

    perf_time[index] = 0;
}

std::string DTimer::GetOutputString(DInt32 index, DTimeUnit unit)
{
    std::string strRet,cs,cs2,indexs;
    std::stringstream ss,sstemp;
    DInt64 c = 0;
    DInt64 c2 = 0;
    std::string tu = "";
    if (unit == DTimeUnit::IN_CLOCK)
    {
        c = perf_time[index];
        tu = "clocks";
    }
    else if (unit == DTimeUnit::IN_NS)
    {
        c = ClockToNSecond(perf_time[index]);
        tu = "ns";
    }
    else if (unit == DTimeUnit::IN_US)
    {
        c = ClockToUSecond(perf_time[index]);
        tu = "us";
    }
    else if (unit == DTimeUnit::IN_MS)
    {
        c = ClockToUSecond(perf_time[index]);
        c2 = c % 1000;
        c = c / 1000;
    }
    else if (unit == DTimeUnit::IN_SEC)
    {
        c = ClockToUSecond(perf_time[index]);
        c2 = c % 1000000;
        c = c / 1000000;
    }

    if (perf_desc[index])
    {
        sstemp << c;
        sstemp >> cs;
        ss << perf_desc[index] << "\tUsing " << cs;
        if (c2 != 0) {
            sstemp << c2;
            sstemp >> cs2;
            ss << "." << cs2;
        }
        ss << " " << tu << "\n";
        strRet = ss.str();
    }
    else
    {
        sstemp << index;
        sstemp >> indexs;
        sstemp.clear();
        sstemp << c;
        sstemp >> cs;
        sstemp.clear();
        ss << "Slot " << indexs << "\tUsing " << cs;
        if (c2 != 0) {
            sstemp << c2;
            sstemp >> cs2;
            ss << "." << cs2;
        }
        ss << " " << tu << "\n";
        strRet = ss.str();
    }
    return strRet;
}

std::string DTimer::GetOutputAllString(DTimeUnit unit)
{
    std::string strRet;
    for (int i = 1; i < PERF_COUNTS; i++)
    {
        if (perf_time[i] != 0)
        {
            std::string strTemp = GetOutputString(i, unit);
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
#if defined(BUILD_FOR_WINDOWS)
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
#elif defined(BUILD_FOR_IOS)
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

