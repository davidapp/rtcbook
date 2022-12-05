#include "DTime.h"

DUInt32 DTime::GetTickCount32()
{
#if defined(BUILD_FOR_WINDOWS)
    D_WARNING_SUPPRESS(28159)
        return ::GetTickCount();
    D_WARNING_UNSUPPRESS()
#elif defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
    static mach_timebase_info_data_t sTimebaseInfo;
    uint64_t machTime = mach_absolute_time();
    if (sTimebaseInfo.denom == 0)
    {
        (void)mach_timebase_info(&sTimebaseInfo);
    }
    // Convert the mach time to mili seconds
    DUInt32 millis = (DUInt32)((machTime / 1000000) * sTimebaseInfo.numer) / sTimebaseInfo.denom;
    return millis;
#elif defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID == 1)
    //TODO
    //SystemClock.uptimeMillis()
    return 0;
#endif
}

//in mili seconds
DUInt64 DTime::GetTickCount64()
{
#if defined(BUILD_FOR_WINDOWS)
    return ::GetTickCount64();
#elif defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
    static mach_timebase_info_data_t sTimebaseInfo;
    uint64_t machTime = mach_absolute_time();
    if (sTimebaseInfo.denom == 0)
    {
        (void)mach_timebase_info(&sTimebaseInfo);
    }
    // Convert the mach time to mili seconds
    DUInt64 millis = (DUInt64)((machTime / 1000000) * sTimebaseInfo.numer) / sTimebaseInfo.denom;
    return millis;
#elif defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID == 1)
    //TODO
    return 0;
#endif
    return 0;
}

DVoid DTime::SleepSec(DUInt32 second)
{
#if defined(BUILD_FOR_WINDOWS)
    ::Sleep(second * 1000);
#else
    sleep(second);
#endif
}
