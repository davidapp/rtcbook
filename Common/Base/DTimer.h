#pragma once

#include "DTypes.h"
#include <string>

enum class DTimeUnit
{
    IN_CLOCK = 0,   //CPU Cycle
    IN_NS = 1,      //10^-9 s
    IN_US = 2,      //10^-6 s
    IN_MS = 3,      //10^-3 s
    IN_SEC = 4      //s
};

class DTimer
{
public:
    static DVoid Init();
    static DVoid Start(DInt32 index, DCStr desc = NULL);
    static DUInt64 Stop(DInt32 index);
    static DUInt64 GetRes(DInt32 index);

public:
    static DVoid Output(DInt32 index, DTimeUnit unit = DTimeUnit::IN_CLOCK);
    static DVoid OutputAll(DTimeUnit unit = DTimeUnit::IN_NS);
    static std::string GetOutputString(DInt32 index, DTimeUnit unit = DTimeUnit::IN_CLOCK);
    static std::string GetOutputAllString(DTimeUnit unit = DTimeUnit::IN_NS);

public:
    static DUInt64 ClockToNSecond(DUInt64 t);
    static DUInt64 ClockToUSecond(DUInt64 t);
    static DUInt64 ClockToMSecond(DUInt64 t);
    static inline DUInt64 GetCycleCount();

public:
    static DVoid SleepSec(DUInt32 second);

public:
    static DUInt64 overhead;    //clock
    static DUInt64 cpuspeed100; //mhz

    D_DISALLOW_COPY_AND_ASSIGN(DTimer)
};
