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
    static std::string BoolToStr(DBool b);
    static std::string UInt8ToStr(DUInt8 c);
    static std::string UInt16ToStr(DUInt16 c, DBool bLE = true);
    static std::string UInt32ToStr(DUInt32 c, DBool bLE = true);
    static std::string UInt64ToStr(DUInt64 c, DBool bLE = true);
    static std::string UInt8ToStr16(DUInt8 c);
    static std::string UInt16ToStr16(DUInt16 c, DBool bLE = true);
    static std::string UInt32ToStr16(DUInt32 c, DBool bLE = true);
    static std::string UInt64ToStr16(DUInt64 c, DBool bLE = true);
    static std::string Int8ToStr(DInt8 c);
    static std::string Int16ToStr(DInt16 c, DBool bLE = true);
    static std::string Int32ToStr(DInt32 c, DBool bLE = true);
    static std::string Int64ToStr(DInt64 c, DBool bLE = true);
    static std::string AddrToStr(void* p);
    static std::string BuffToStr(void* p, DUInt32 len);

public:
    static std::string DumpBitmapFileHeader(void* pFileHeader); // DBITMAPFILEHEADER
    static std::string DumpBitmapInfoHeader(void* pInfoHeader); // DBITMAPINFOHEADER

public:
#if defined(BUILD_FOR_WINDOWS)
    static std::string RECTToStr(RECT rc);
    static std::string GUIDToStr(GUID id);
    static std::string Dump_AM_MEDIA_TYPE(void* amt);
    static std::string Dump_VIDEO_STREAM_CONFIG_CAPS(void* vscc);
    static std::string Dump_VIDEOINFOHEADER(void* vih);
#endif

    D_DISALLOW_ALL_DEFAULT(DUtil)
};
