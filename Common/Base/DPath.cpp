#include "DPath.h"
#include "Base/DXP.h"

#if defined(BUILD_FOR_WINDOWS)
#include <shlobj.h>
#elif defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
#include "OCBridge/OCPath.h"
#elif defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID==1)

#elif defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1)
#include "OCBridge/OCPath.h"
#elif defined(BUILD_FOR_LINUX) && (BUILD_FOR_LINUX==1)

#endif


std::string DPath::GetParent(std::string& path)
{
    std::string strRet = path;
    DInt32 index = strRet.rfind(D_PATH_SLASH) + 1;
    strRet = strRet.substr(0, index);
    return strRet;
}


std::string DPath::GetExePath()
{
#if defined(BUILD_FOR_WINDOWS)
    WCHAR pstr[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, pstr, MAX_PATH);
#else
    strRet = L2W(L"/");
#endif
    std::wstring wstr = pstr;
    return DXP::ws2s(wstr);
}

std::string DPath::GetExeDir()
{
    std::string strPath = DPath::GetExePath();
    return DPath::GetParent(strPath);
}

#if defined(BUILD_FOR_WINDOWS)

std::string DPath::GetSystemDir()
{
    WCHAR pstr[MAX_PATH];
    ::GetSystemDirectory(pstr, MAX_PATH);
    std::wstring strRet = pstr;
    return DXP::ws2s(strRet);
}

std::string DPath::GetWindowsDir()
{
    WCHAR pstr[MAX_PATH];
    ::GetWindowsDirectoryW(pstr, MAX_PATH);
    std::wstring strRet = pstr;
    return DXP::ws2s(strRet);
}

std::string DPath::GetMyDocumentDir()
{
    return GetSHDir(CSIDL_PERSONAL);
}

std::string DPath::GetFontDir()
{
    return GetSHDir(CSIDL_FONTS);
}

std::string DPath::GetAppDataDir()
{
    return GetSHDir(CSIDL_APPDATA);
}

std::string DPath::GetDesktopDir()
{
    return GetSHDir(CSIDL_DESKTOP);
}

/*
CSIDL_BITBUCKET
CSIDL_CONTROLS
CSIDL_DESKTOP
CSIDL_DESKTOPDIRECTORY
CSIDL_DRIVES
CSIDL_FONTS
CSIDL_NETHOOD
CSIDL_NETWORK
CSIDL_PERSONAL
CSIDL_PRINTERS
CSIDL_PROGRAMS
CSIDL_RECENT
CSIDL_SENDTO
CSIDL_STARTMENU
CSIDL_STARTUP
CSIDL_TEMPLATES
*/

std::string DPath::GetSHDir(DInt32 nKind)
{
    DWChar m_lpszDefaultDir[MAX_PATH] = { 0 };
    DWChar szDocument[MAX_PATH] = { 0 };

    LPITEMIDLIST pidl = NULL;
    HRESULT hRes = SHGetSpecialFolderLocation(NULL, nKind, &pidl);
    if (FAILED(hRes))
    {
        return "";
    }

    if (pidl && SHGetPathFromIDList(pidl, (LPWSTR)szDocument))
    {
        GetShortPathName((LPWSTR)szDocument, (LPWSTR)m_lpszDefaultDir, _MAX_PATH);
    }
    std::wstring strRet = szDocument;
    return DXP::ws2s(strRet);
}


#elif defined(BUILD_FOR_IOS) || defined(BUILD_FOR_MAC)
std::string DPath::GetHomeDir()
{
    std::string strRet;
    strRet = getHomeDir();
    return strRet;
}

std::string DPath::GetDocDir()
{
    std::string strRet;
    strRet = getDocDir();
    return strRet;
}

std::string DPath::GetCacheDir()
{
    std::string strRet;
    strRet = getCacheDir();
    return strRet;
}

std::string DPath::GetTempDir()
{
    std::string strRet;
    strRet = getTempDir();
    return strRet;
}

std::string DPath::GetMoivesDir()
{
    std::string strRet;
    strRet = getMoivesDir();
    return strRet;
}

std::string DPath::GetMusicDir()
{
    std::string strRet;
    strRet = getMusicDir();
    return strRet;
}

std::string DPath::GetPicDir()
{
    std::string strRet;
    strRet = getPicDir();
    return strRet;
}

std::string DPath::GetDownloadDir()
{
    std::string strRet;
    strRet = getDownloadDir();
    return strRet;
}
#endif


