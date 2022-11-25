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
