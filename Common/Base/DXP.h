#pragma once

#include "DTypes.h"
#include <string>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

class DXP
{
public:
    static DCStr GetOSName();
    static DVoid Print(std::string str);

public:
    static std::string ws2s(const std::wstring& ws);
    static std::wstring s2ws(const std::string& s);
    static std::string& replace_str(std::string& str, const std::string& to_replaced, const std::string& newchars);

public:
    static DInt32 isatof(DInt32 c);
    static DInt32 isAtoF(DInt32 c);

public:
    static DUInt16 Swap16(DUInt16 h);
    static DUInt32 Swap32(DUInt32 h);
    static DUInt64 Swap64(DUInt64 h);
public:
    static DVoid* memcpy(DVoid* dest, const DVoid* src, DSizeT count);

    D_DISALLOW_ALL_DEFAULT(DXP)
};
