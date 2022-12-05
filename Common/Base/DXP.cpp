#include "DXP.h"

DCStr DXP::GetOSName()
{
#if defined(BUILD_FOR_WINDOWS)
    return "Windows";
#elif defined(BUILD_FOR_MAC)
    return "MacOS";
#elif defined(BUILD_FOR_IOS)
    return "iOS";
#elif defined(BUILD_FOR_ANDROID)
    return "Android";
#else
    return "Linux";
#endif
}

DVoid DXP::Print(std::string str)
{
#if defined(BUILD_FOR_WINDOWS)
    ::WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), str.c_str(), str.size(), NULL, NULL);
#else
    write(STDOUT_FILENO, str.c_str(), str.size());
#endif
}

DVoid* DXP::memcpy(DVoid* dest, const DVoid* src, DSizeT count)
{
#if defined(BUILD_FOR_WINDOWS)
    memcpy_s(dest, count, src, count);
#else
    ::memcpy(dest, src, count);
#endif
    return nullptr;
}

std::string DXP::ws2s(const std::wstring& ws)
{
    size_t convertedChars = 0;
    std::string curLocale = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "chs");
    const wchar_t* _Source = ws.c_str();
    size_t _Dsize = ws.size() * 2 + 1;
    char* _Dest = new char[_Dsize];
    memset(_Dest, 0, _Dsize);
#if defined(BUILD_FOR_WINDOWS)
    wcstombs_s(&convertedChars, _Dest, _Dsize, _Source, ws.size() * 2);
#else
    D_UNUSED(convertedChars);
    wcstombs(_Dest, _Source, MB_CUR_MAX);
#endif
    std::string result = _Dest;
    delete[]_Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}

std::wstring DXP::s2ws(const std::string& s)
{
    size_t convertedChars = 0;
    std::string curLocale = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "chs");
    const char* source = s.c_str();
    size_t charNum = sizeof(char) * s.size() + 1;
    wchar_t* dest = new wchar_t[charNum];
#if defined(BUILD_FOR_WINDOWS)
    mbstowcs_s(&convertedChars, dest, charNum, source, _TRUNCATE);
#else
    D_UNUSED(convertedChars);
    mbstowcs(dest, source, MB_CUR_MAX);
#endif
    std::wstring result = dest;
    delete[] dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}

std::string& DXP::replace_str(std::string& str, const std::string& to_replaced, const std::string& newchars)
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

DInt32 DXP::isatof(DInt32 c)
{
    return (c >= 'a' && c <= 'f');
}

DInt32 DXP::isAtoF(DInt32 c)
{
    return (c >= 'A' && c <= 'F');
}

DUInt16 DXP::Swap16(DUInt16 h)
{
    DUInt16 ret;
    DByte* p1 = (DByte*)&h;
    DByte* p2 = (DByte*)&ret;
    p2[0] = p1[1];
    p2[1] = p1[0];
    return ret;
}

DUInt32 DXP::Swap32(DUInt32 h)
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

DUInt64 DXP::Swap64(DUInt64 h)
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

std::string DXP::BoolToStr(DBool b)
{
    if (b) return "true";
    return "false";
}

std::string DXP::UInt8ToStr(DUInt8 c)
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

std::string DXP::UInt16ToStr(DUInt16 c, DBool bLE)
{
    char buf[10] = {};
    if (!bLE) {
        c = DXP::Swap16(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%u", c);
#else
    sprintf(buf, "%u", c);
#endif
    std::string str = buf;
    return str;
}

std::string DXP::UInt32ToStr(DUInt32 c, DBool bLE)
{
    char buf[20] = {};
    if (!bLE) {
        c = DXP::Swap32(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "%u", c);
#else
    sprintf(buf, "%u", c);
#endif
    std::string str = buf;
    return str;
}

std::string DXP::UInt64ToStr(DUInt64 c, DBool bLE)
{
    char buf[40] = {};
    if (!bLE) {
        c = DXP::Swap64(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 40, "%llu", c);
#else
    sprintf(buf, "%llu", c);
#endif
    std::string str = buf;
    return str;
}

std::string DXP::UInt8ToStr16(DUInt8 c)
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

std::string DXP::UInt16ToStr16(DUInt16 c, DBool bLE)
{
    char buf[10] = {};
    if (!bLE) {
        c = DXP::Swap16(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "0x%02x %02x", c >> 8, c & 0x00FF);
#else
    sprintf(buf, "0x%02x %02x", c >> 8, c & 0x00FF);
#endif
    std::string str = buf;
    return str;
}

std::string DXP::UInt32ToStr16(DUInt32 c, DBool bLE)
{
    char buf[20] = {};
    if (!bLE) {
        c = DXP::Swap32(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "0x%02x %02x %02x %02x", c >> 24, (c >> 16) & 0x000000FF, (c >> 8) & 0x000000FF, c & 0x000000FF);
#else
    sprintf(buf, "0x%02x %02x %02x %02x", c >> 24, (c >> 16) & 0x000000FF, (c >> 8) & 0x000000FF, c & 0x000000FF);
#endif
    std::string str = buf;
    return str;
}

std::string DXP::Int8ToStr(DInt8 c)
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

std::string DXP::Int16ToStr(DInt16 c, DBool bLE)
{
    char buf[10] = {};
    if (!bLE) {
        c = DXP::Swap16(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%d", c);
#else
    sprintf(buf, "%d", c);
#endif
    std::string str = buf;
    return str;
}

std::string DXP::Int32ToStr(DInt32 c, DBool bLE)
{
    char buf[20] = {};
    if (!bLE) {
        c = DXP::Swap32(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "%d", c);
#else
    sprintf(buf, "%d", c);
#endif
    std::string str = buf;
    return str;
}

std::string DXP::Int64ToStr(DInt64 c, DBool bLE)
{
    char buf[40] = {};
    if (!bLE) {
        c = DXP::Swap64(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 40, "%lld", c);
#else
    sprintf(buf, "%lld", c);
#endif
    std::string str = buf;
    return str;
}

std::string DXP::AddrToStr(void* p)
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

DInt32 DXP::StrToInt32(std::string str)
{
    return atoi(str.c_str());
}

DInt32 DXP::Str16ToInt32(std::wstring wstr)
{
    return atoi(DXP::ws2s(wstr).c_str());
}

DUInt32 DXP::GetTickCount32()
{
#if defined(BUILD_FOR_WINDOWS)
    D_WARNING_SUPPRESS(28159)
        return ::GetTickCount();
    D_WARNING_UNSUPPRESS()
#elif defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
    static mach_timebase_info_data_t sTimebaseInfo;
    uint64_t machTime = mach_absolute_time();
    if (sTimebaseInfo.denom == 0)
    {
        (void)mach_timebase_info(&sTimebaseInfo);
    }
    // Convert the mach time to mili seconds
    DUInt32 millis = (DUInt32)((machTime / 1000000) * sTimebaseInfo.numer) / sTimebaseInfo.denom;
    return millis;
#elif defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID == 1)
    //TODO
    //SystemClock.uptimeMillis()
    return 0;
#endif
}

//in mili seconds
DUInt64 DXP::GetTickCount64()
{
#if defined(BUILD_FOR_WINDOWS)
    return ::GetTickCount64();
#elif defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
    static mach_timebase_info_data_t sTimebaseInfo;
    uint64_t machTime = mach_absolute_time();
    if (sTimebaseInfo.denom == 0)
    {
        (void)mach_timebase_info(&sTimebaseInfo);
    }
    // Convert the mach time to mili seconds
    DUInt64 millis = (DUInt64)((machTime / 1000000) * sTimebaseInfo.numer) / sTimebaseInfo.denom;
    return millis;
#elif defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID == 1)
    //TODO
    return 0;
#endif
    return 0;
}

DVoid DXP::SleepSec(DUInt32 second)
{
#if defined(BUILD_FOR_WINDOWS)
    ::Sleep(second * 1000);
#else
    sleep(second);
#endif
}