#pragma once

#include "DConfig.h"
#include "DTypes.h"
#include <string>

class DPath
{
public:
    static std::string GetParent(std::string& path);
    static std::string GetExePath();
    static std::string GetExeDir();

#if defined(BUILD_FOR_WINDOWS)
    static std::string GetSystemDir();
    static std::string GetWindowsDir();
    static std::string GetMyDocumentDir();
    static std::string GetDesktopDir();
    static std::string GetFontDir();
    static std::string GetAppDataDir();
    static std::string GetTempDir();
    static std::string GetTempFileName();

#elif defined(BUILD_FOR_IOS)
    static std::string GetHomeDir();
    static std::string GetDocDir();
    static std::string GetCacheDir();
    static std::string GetTempDir();
    static std::string GetMoivesDir();
    static std::string GetMusicDir();
    static std::string GetPicDir();
    static std::string GetDownloadDir();
#elif defined(BUILD_FOR_ANDROID)
#elif defined(BUILD_FOR_MAC)
    static std::string GetHomeDir();
    static std::string GetDocDir();
    static std::string GetCacheDir();
    static std::string GetTempDir();
    static std::string GetMoivesDir();
    static std::string GetMusicDir();
    static std::string GetPicDir();
    static std::string GetDownloadDir();
#elif defined(BUILD_FOR_LINUX)
#endif

protected:
#if defined(BUILD_FOR_WINDOWS)
    static std::string GetSHDir(DInt32 nKind);
#endif

    D_DISALLOW_ALL_DEFAULT(DPath)
};
