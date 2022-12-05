#pragma once

#include "DConfig.h"
#include "DTypes.h"

class DTime
{
public:
    static DUInt32 GetTickCount32();            //in MILI second since system started, up to 49.7 days on Win32
    static DUInt64 GetTickCount64();            //in MILI second since system started
    static DVoid   SleepSec(DUInt32 second);
};

