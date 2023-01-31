#include "DTime.h"
#include <chrono>

DUInt64 DTime::GetTicks()
{
    auto now_time = std::chrono::steady_clock::now();
    DUInt64 nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now_time.time_since_epoch()).count();
    return nano_time;
}

DUInt64 DTime::GetTimeStamp()
{
    auto now_time = std::chrono::system_clock::now();
    DUInt64 nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now_time.time_since_epoch()).count();
    return nano_time;
}

DUInt64 DTime::GetTimeStamp_ntp()
{
    auto now_time = std::chrono::system_clock::now();
    DUInt64 nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now_time.time_since_epoch()).count();
    DUInt64 nano_ntp = nano_time + 2208988800UL * 1000000000LL;
    return nano_ntp;
}
