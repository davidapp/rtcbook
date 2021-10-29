﻿#include "DUtil.h"
#include <locale>
#include "File/DBmpFile.h"
#include "Base/DBuffer.h"
#include "Video/VideoDefines.h"

#if defined(BUILD_FOR_WINDOWS)
#include <dshow.h>
#include "Video/WinDSCamera.h"
#endif

std::string DUtil::ws2s(const std::wstring& ws)
{
    size_t convertedChars = 0;
    std::string curLocale = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "chs");
    const wchar_t* _Source = ws.c_str();
    size_t _Dsize = 2 * ws.size() + 1;
    char* _Dest = new char[_Dsize];
    memset(_Dest, 0, _Dsize);
    wcstombs_s(&convertedChars, _Dest, _Dsize, _Source, _Dsize-1);
    std::string result = _Dest;
    delete[]_Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}

std::wstring DUtil::s2ws(const std::string& s)
{
    size_t convertedChars = 0;
    std::string curLocale = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "chs");
    const char* source = s.c_str();
    size_t charNum = sizeof(char) * s.size() + 1;
    wchar_t* dest = new wchar_t[charNum];
    mbstowcs_s(&convertedChars, dest, charNum, source, _TRUNCATE);
    std::wstring result = dest;
    delete[] dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}

std::string& DUtil::replace_str(std::string& str, const std::string& to_replaced, const std::string& newchars)
{
    for (std::string::size_type pos(0); pos != std::string::npos; pos += newchars.length())
    {
        pos = str.find(to_replaced, pos);
        if (pos != std::string::npos)
            str.replace(pos, to_replaced.length(), newchars);
        else
            break;
    }
    return str;
}

DInt32 DUtil::isatof(DInt32 c)
{
    return (c >= 'a' && c <= 'f');
}

DInt32 DUtil::isAtoF(DInt32 c)
{
    return (c >= 'A' && c <= 'F');
}

DUInt16 DUtil::Swap16(DUInt16 h)
{
    DUInt16 ret;
    DByte* p1 = (DByte*)&h;
    DByte* p2 = (DByte*)&ret;
    p2[0] = p1[1];
    p2[1] = p1[0];
    return ret;
}

DUInt32 DUtil::Swap32(DUInt32 h)
{
    DUInt32 ret;
    DByte* p1 = (DByte*)&h;
    DByte* p2 = (DByte*)&ret;
    p2[0] = p1[3];
    p2[1] = p1[2];
    p2[2] = p1[1];
    p2[3] = p1[0];
    return ret;
}

DUInt64 DUtil::Swap64(DUInt64 h)
{
    DUInt64 ret;
    DByte* p1 = (DByte*)&h;
    DByte* p2 = (DByte*)&ret;
    p2[0] = p1[7];
    p2[1] = p1[6];
    p2[2] = p1[5];
    p2[3] = p1[4];
    p2[4] = p1[3];
    p2[5] = p1[2];
    p2[6] = p1[1];
    p2[7] = p1[0];
    return ret;
}

std::string DUtil::BoolToStr(DBool b)
{
    if (b) return "true";
    return "false";
}

std::string DUtil::UInt8ToStr(DUInt8 c)
{
    char buf[10] = {};
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%u", c);
#else
    sprintf(buf, "%u", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt16ToStr(DUInt16 c, DBool bLE)
{
    char buf[10] = {};
    if (!bLE) {
        c = Swap16(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%u", c);
#else
    sprintf(buf, "%u", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt32ToStr(DUInt32 c, DBool bLE)
{
    char buf[20] = {};
    if (!bLE) {
        c = Swap32(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "%u", c);
#else
    sprintf(buf, "%u", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt64ToStr(DUInt64 c, DBool bLE)
{
    char buf[40] = {};
    if (!bLE) {
        c = Swap64(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 40, "%llu", c);
#else
    sprintf(buf, "%llu", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt8ToStr16(DUInt8 c)
{
    char buf[10] = {};
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "0x%02x", c);
#else
    sprintf(buf, "0x%02x", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt16ToStr16(DUInt16 c, DBool bLE)
{
    char buf[10] = {};
    if (!bLE) {
        c = Swap16(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "0x%02x %02x", c >> 8, c & 0x00FF);
#else
    sprintf(buf, "0x%02x %02x", c >> 8, c & 0x00FF);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt32ToStr16(DUInt32 c, DBool bLE)
{
    char buf[20] = {};
    if (!bLE) {
        c = Swap32(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "0x%02x %02x %02x %02x", c >> 24, (c >> 16) & 0x000000FF, (c >> 8) & 0x000000FF, c & 0x000000FF);
#else
    sprintf(buf, "0x%02x %02x %02x %02x", c >> 24, (c >> 16) & 0x000000FF, (c >> 8) & 0x000000FF, c & 0x000000FF);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt64ToStr16(DUInt64 c, DBool bLE)
{
    char buf[40] = {};
    if (!bLE) {
        c = Swap64(c);
    }
    DBuffer bufTemp(&c, 8);
    std::string str = bufTemp.ToHexString();
    return str;
}

std::string DUtil::Int8ToStr(DInt8 c)
{
    char buf[10] = {};
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%d", c);
#else
    sprintf(buf, "%d", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::Int16ToStr(DInt16 c, DBool bLE)
{
    char buf[10] = {};
    if (!bLE) {
        c = Swap16(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%d", c);
#else
    sprintf(buf, "%d", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::Int32ToStr(DInt32 c, DBool bLE)
{
    char buf[20] = {};
    if (!bLE) {
        c = Swap32(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "%d", c);
#else
    sprintf(buf, "%d", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::Int64ToStr(DInt64 c, DBool bLE)
{
    char buf[40] = {};
    if (!bLE) {
        c = Swap64(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 40, "%lld", c);
#else
    sprintf(buf, "%lld", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::AddrToStr(void* p)
{
    char buf[20] = {};
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "0x%p", p);
#else
    sprintf(buf, "0x%p", p);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::BuffToStr(void* p, DUInt32 len)
{
    DBuffer buf(p, len);
    std::string ret = buf.ToHexString();
    return ret;
}

std::string DUtil::FRArrayToStr(void* p, DUInt32 len)
{
    DUInt64* ptr = (DUInt64*)p;
    std::string ret;
    ret += "[";
    for (DUInt32 i = 0; i < len; i++) {
        ret += UInt32ToStr((DUInt32)(10000000 / ptr[i]));
        if (i != len - 1) {
            ret += ", ";
        }
    }
    ret += "]\r\n";
    return ret;
}
