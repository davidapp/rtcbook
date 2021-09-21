#pragma once

#include "DTypes.h"

class DAPI DNet
{
public:
	static DBool Init();
	static DBool UnInit();

public:
    DUInt16 Swap16(DUInt16 h);
    DUInt32 Swap32(DUInt32 h);
};
