#pragma once

#include "DTypes.h"

class DTime
{
public:
    static DUInt64 GetTicks();
    static DUInt64 GetTimeStamp();
    static DUInt64 GetTimeStamp_ntp();
};

