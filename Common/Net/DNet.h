#pragma once

#include "DTypes.h"

class DNet
{
public:
	static DBool Init();
	static DBool UnInit();

public:
    static DUInt16 Swap16(DUInt16 h);
    static DUInt32 Swap32(DUInt32 h);
    static DUInt64 Swap64(DUInt64 h);
};
