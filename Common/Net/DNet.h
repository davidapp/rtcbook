#pragma once

#include "DTypes.h"
#include <string>

class DNet
{
public:
    static DBool Init();
    static DBool UnInit();
    static DUInt32 GetLastNetError();
    static std::string GetLastNetErrorStr(DUInt32 errCode);
public:
    static DChar*  StrToUChar(DChar* pStr, DByte* pByte);
    static DUInt32 IPStrToUint32(DCStr strIP);
    static std::string UInt32ToIPStr(DUInt32 uip);
};
