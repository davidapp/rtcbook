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
    return nullptr;
#else
    return memcpy(dest, src, count);
#endif
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
