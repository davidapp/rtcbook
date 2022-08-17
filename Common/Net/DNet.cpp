#include "DNet.h"
#include "Base/DUtil.h"
#include "Base/DMsgQueue.h"

#if defined(BUILD_FOR_WINDOWS)
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
WSADATA g_wsaData;
#define DBadSocket INVALID_SOCKET
#define DSockError SOCKET_ERROR
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#define DBadSocket -1
#define DSockError -1
#endif

DHandle g_connqueue;
DHandle g_sendqueue;

DVoid* DX86_STDCALL ConnHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    if (msg == DM_NET_CONN)
    {
        DConnData* pData = (DConnData*)para1;
        DTCPClient* sock = (DTCPClient*)para2;
        DTCPClientSink* pSink = pData->pSink;
#if defined(BUILD_FOR_WINDOWS)
        SOCKADDR_IN addr;
        memset(&addr, 0, sizeof(SOCKADDR_IN));
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, pData->strIP.c_str(), &addr.sin_addr.s_addr);
        addr.sin_port = htons(pData->wPort);
#else
        sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, pData->strIP, &addr.sin_addr);
        addr.sin_port = htons(pData->wPort);
#endif
        if (pSink)
        {
            pSink->OnConnecting(sock, pData->strIP, pData->wPort);
        }
#if defined(BUILD_FOR_WINDOWS)
        if (connect(pData->sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
#else
        if (connect(pData->sock, (sockaddr*)&addr, sizeof(sockaddr_in)) == DSockError)
#endif
        {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            if (pSink)
            {
                pSink->OnConnectError(sock, errCode, strReason);
            }
            delete pData;
            return NULL;
        }
        if (pSink)
        {
            pSink->OnConnectOK(sock);
        }

        delete pData;
    }
    return NULL;
}

DVoid* DX86_STDCALL SendHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    if (msg == DM_NET_SEND)
    {
        DSendData* pData = (DSendData*)para1;
        DTCPClient* sock = (DTCPClient*)para2;
        DBuffer buf;
        buf.Attach(pData->buffer);
        DTCPDataSink* pSink = pData->pSink;
        DChar* pStart = (DChar*)pData->buffer;
        DUInt32 size = buf.GetSize();
        //DUInt32 sizeHead = DNet::H2N(size);
        if (pSink)
        {
            pSink->OnPreSend(sock, buf);
        }
        DUInt32 sent = 0;
        while (sent < size)
        {
            DInt32 ret = (DInt32)send(pData->sock, pStart, size - sent, 0);//MSG_DONTROUTE MSG_OOB
            if (ret == DSockError)
            {
                DUInt32 errCode = DNet::GetLastNetError();
                std::string strReasonA = DNet::GetLastNetErrorStr(errCode);
                if (pSink)
                {
                    pSink->OnSendError(sock, errCode, strReasonA);
                }
                delete pData;
                return NULL;
            }
            sent += ret;
            pStart += ret;
        }
        if (pSink)
        {
            pSink->OnSendOK(sock);
        }
        delete pData;
    }
    return NULL;
}


DVoid* DX86_STDCALL SendToHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    /*
    if (msg == DM_NET_SEND)
    {
        DSendToData* pData = (DSendToData*)para1;
        DUDPClient* sock = (DUDPClient*)para2;
        DBuffer buf;
        buf.Attach(pData->buffer);
        DUDPDataSink* pSink = pData->pSink;
        DChar* pStart = (DChar*)buf.GetBuf();
        DUInt32 size = buf.GetSize();
        if (pSink)
        {
            pSink->OnPreSend(sock, (DByte*)pStart, size);
        }
#if defined(BUILD_FOR_WINDOWS)
        SOCKADDR_IN ReceiverAddr = { 0 };
#else
        struct sockaddr_in ReceiverAddr = { 0 };
#endif
        ReceiverAddr.sin_family = AF_INET;
        ReceiverAddr.sin_port = htons(pData->wPort);
        inet_pton(AF_INET, pData->strIP, &ReceiverAddr.sin_addr.s_addr);

        DUInt32 sent = 0;
        while (sent < size)
        {
#if defined(BUILD_FOR_WINDOWS)
            DInt32 ret = sendto(pData->sock, pStart, size - sent, 0, (SOCKADDR*)&ReceiverAddr, sizeof(ReceiverAddr));
#else
            DInt32 ret = (DInt32)sendto(pData->sock, pStart, size - sent, 0, (sockaddr*)&ReceiverAddr, sizeof(ReceiverAddr));
#endif
            if (ret == DSockError)
            {
                DUInt32 errCode = DError::GetLastNetError();
                DStringA strReasonA = DError::GetLastNetErrorStr();
                if (pSink)
                {
                    pSink->OnSendError(sock, errCode, strReasonA);
                }
                return NULL;
            }
            sent += ret;
            pStart += ret;
        }
        if (pSink)
        {
            pSink->OnSendOK(sock);
            sock->Recv();
        }
        DStringA strA;
        strA.Attach(pData->strIP);

        delete pData;
    }
    */
    return NULL;
}

DBool DNet::Init()
{
#if defined(BUILD_FOR_WINDOWS)
    if (WSAStartup(MAKEWORD(2, 2), &g_wsaData) != 0)
    {
        return false;
    }
#endif
    g_connqueue = DMsgQueue::Create("ConnQueue", 100);
    DMsgQueue::AddHandler(g_connqueue, ConnHandler);

    g_sendqueue = DMsgQueue::Create("SendQueue", 100);
    DMsgQueue::AddHandler(g_sendqueue, SendHandler);
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

    return DUtil::ws2s(strRet);
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
    sprintf(buf, "%d.%d.%d.%d", pByte[0], pByte[1], pByte[2], pByte[3]);
    str = buf;
    return str;
}

DHandle DNet::GetConnQueue()
{
    return g_connqueue;
}

DHandle DNet::GetSendQueue()
{
    return g_sendqueue;
}

DVoid DNet::AddConnReq(DTCPClient* sock, std::string strIP, DUInt16 wPort)
{
    DConnData* pData = new DConnData();
    pData->sock = sock->m_sock;
    pData->strIP = strIP.c_str();
    pData->wPort = wPort;
    pData->pSink = sock->m_pConnSink;
    DMsgQueue::PostQueueMsg(g_connqueue, DM_NET_CONN, pData, sock);
}

DVoid DNet::AddSendReq(DTCPClient* sock, DBuffer buffer)
{
    DSendData* pData = new DSendData();
    pData->sock = sock->m_sock;
    pData->buffer = buffer.GetBuf();
    pData->pSink = sock->m_pDataSink;
    buffer.Detach();
    DMsgQueue::PostQueueMsg(g_sendqueue, DM_NET_SEND, pData, sock);
}
