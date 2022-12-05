#include "DNet.h"
#include "Base/DXP.h"
#include "Base/DMsgQueue.h"

#if defined(BUILD_FOR_WINDOWS)
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
WSADATA g_wsaData;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#endif

DBool DNet::Init()
{
#if defined(BUILD_FOR_WINDOWS)
    if (WSAStartup(MAKEWORD(2, 2), &g_wsaData) != 0)
    {
        return false;
    }
#endif
    return true;
}

DBool DNet::UnInit()
{
#if defined(BUILD_FOR_WINDOWS)
    if (WSACleanup() == SOCKET_ERROR)
    {
        return false;
    }
#endif
    return true;
}

DUInt32 DNet::GetLastNetError()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    return ::WSAGetLastError();
#else
    return errno;
#endif
}

std::string DNet::GetLastNetErrorStr(DUInt32 errCode)
{
#if defined(BUILD_FOR_WINDOWS)
    DWORD systemLocale = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    HLOCAL hlocal = NULL;
    BOOL fOk = ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, (DWORD)errCode, systemLocale, (PWSTR)&hlocal, 0, NULL);

    if (!fOk)
    {
        HMODULE hDll = ::LoadLibraryExW(L"netmsg.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
        BOOL fOk = FALSE;
        if (hDll)
        {
            fOk = ::FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                hDll, errCode, systemLocale, (PWSTR)&hlocal, 0, NULL);
            FreeLibrary(hDll);
        }
    }

    std::wstring strRet;
    if (fOk && hlocal)
    {
        LPVOID ptr = ::LocalLock(hlocal);
        if (ptr != NULL) {
            strRet = (DCWStr)ptr;
            ::LocalFree(hlocal);
        }
    }

    return DXP::ws2s(strRet);
#else
    std::string strRet;
    strRet = strerror(errCode);
    return strRet;
#endif
}

DChar* DNet::StrToUChar(DChar* pStr, DByte* pByte)
{
    if (pStr == nullptr) return nullptr;

    DBool bValid = false;
    DUInt32 val = 0;
    while (isdigit(*pStr))
    {
        bValid = true;
        val = val * 10 + *pStr - '0';
        pStr++;
    }
    if (bValid)
    {
        if (val <= 255)
        {
            *pByte = val;
            return pStr;
        }
    }
    return nullptr;
}

DUInt32 DNet::IPStrToUint32(DCStr strIP)
{
    DUInt32 ret = 0;
    DByte* pByte = (DByte*)&ret;
    DChar* pStart = (DChar*)strIP;
    pStart = StrToUChar(pStart, pByte);
    if ((pStart == NULL) || (*pStart != '.')) return 0;
    pStart = StrToUChar(pStart + 1, pByte + 1);
    if ((pStart == NULL) || (*pStart != '.')) return 0;
    pStart = StrToUChar(pStart + 1, pByte + 2);
    if ((pStart == NULL) || (*pStart != '.')) return 0;
    pStart = StrToUChar(pStart + 1, pByte + 3);
    if (pStart == NULL || (*pStart != '\0')) return 0;
    return ret;
}

std::string DNet::UInt32ToIPStr(DUInt32 uip)
{
    DChar buf[30] = {};
    std::string str;
    DByte* pByte = (DByte*)&uip;
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 30, "%d.%d.%d.%d", pByte[0], pByte[1], pByte[2], pByte[3]);
#else
    sprintf(buf, "%d.%d.%d.%d", pByte[0], pByte[1], pByte[2], pByte[3]);
#endif
    str = buf;
    return str;
}

