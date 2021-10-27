﻿#pragma once

#include "DTypes.h"
#include <string>

class DUtil
{
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
    static std::string UInt8ToStr(DUInt8 c);
    static std::string UInt16ToStr(DUInt16 c, DBool bLE = false);
    static std::string UInt32ToStr(DUInt32 c, DBool bLE=false);
    static std::string UInt8ToStr16(DUInt8 c);
    static std::string UInt16ToStr16(DUInt16 c, DBool bLE = false);
    static std::string UInt32ToStr16(DUInt32 c, DBool bLE = false);

public:
    static std::string DumpBitmapFileHeader(void* pFileHeader); // DBITMAPFILEHEADER
    static std::string DumpBitmapInfoHeader(void* pFileHeader); // DBITMAPINFOHEADER

    D_DISALLOW_ALL_DEFAULT(DUtil)
};
