#include "DTCPClient.h"
#include "Base/DUtil.h"
#include "Net/DNet.h"

#if defined(BUILD_FOR_WINDOWS)
#include <winsock2.h>
#define DBadSocket INVALID_SOCKET
#define DSockError SOCKET_ERROR
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h>     //IPPROTO_TCP
#include <unistd.h>
#include <string.h>
#define DBadSocket -1
#define DSockError -1
#endif


DTCPSocket::DTCPSocket()
{
    m_sock = DBadSocket;
}

DTCPSocket::DTCPSocket(DSocket sock)
{
    m_sock = sock;
}

DTCPSocket::~DTCPSocket()
{
    m_sock = DBadSocket;
}

DBool DTCPSocket::Create(DBool bIPv6)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    if (m_sock == DBadSocket)
    {
        m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_sock == DBadSocket)
        {
            return false;
        }
    }
#else
    if (m_sock == DBadSocket)
    {
        m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_sock == DBadSocket)
        {
            return false;
        }
    }
#endif
    return true;
}

DVoid DTCPSocket::Close()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    if (m_sock != DBadSocket)
    {
        closesocket(m_sock);
        m_sock = DBadSocket;
    }
#else
    if (m_sock != DBadSocket)
    {
        close(m_sock);
        m_sock = DBadSocket;
    }
#endif
}

DVoid DTCPSocket::Attach(DSocket sock)
{
    m_sock = sock;
}

DVoid DTCPSocket::Detach()
{
    m_sock = DBadSocket;
}

DVoid DTCPSocket::Renew()
{
    Close();
    Create();
}

DBool DTCPSocket::operator==(const DTCPSocket sock)
{
    if (m_sock == sock.m_sock) return true;
    return false;
}

DBool DTCPSocket::IsValid()
{
    return m_sock != DBadSocket;
}

DBool DTCPSocket::Bind(DUInt16 port)
{
    SOCKADDR_IN InternetAddr = {};
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(port);
    int ret = bind(m_sock, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr));
    if (ret != 0) {
        return false;
    }
    return true;
}

DBool DTCPSocket::Listen(DInt32 backlog)
{
    int ret = listen(m_sock, backlog);
    if (ret != 0) {
        return false;
    }
    return true;
}

DTCPSocket DTCPSocket::Accept()
{
    DTCPSocket sockRet;
    SOCKET NewConnection = accept(m_sock, 0, 0);
    sockRet.Attach(NewConnection);
    return sockRet;
}

DInt32 DTCPSocket::Shutdown(DInt32 how)
{
    return shutdown(m_sock, how);
}

DBool DTCPSocket::SyncConnect(DCStr strIP, DUInt16 wPort)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(SOCKADDR_IN));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = DNet::IPStrToUint32(strIP);
    addr.sin_port = DUtil::Swap16(wPort);
    if (connect(m_sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        DUInt32 errCode = DNet::GetLastNetError();
        std::string strReasonA = DNet::GetLastNetErrorStr(errCode);
        return false;
    }
#else
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, strIP, &addr.sin_addr.s_addr);
    addr.sin_port = htons(wPort);
    if (connect(m_sock, (sockaddr*)&addr, sizeof(struct sockaddr_in)) == DSockError)
    {
        DUInt32 errCode = DNet::GetLastNetError();
        D_UNUSED(errCode);
        std::string strReasonA = DNet::GetLastNetErrorStr();
        return false;
    }
#endif
    return true;
}

DBool DTCPSocket::SyncSend(DBuffer buf)
{
    DUInt32 sent = 0;
    DChar* pStart = (DChar*)buf.GetBuf();
    DUInt32 sizeAll = buf.GetSize();
    while (sent < sizeAll)
    {
        DInt32 ret = (DInt32)send(m_sock, pStart, sizeAll - sent, 0);//MSG_DONTROUTE MSG_OOB
        if (ret == DSockError)
        {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReasonA = DNet::GetLastNetErrorStr(errCode);
            return false;
        }
        sent += ret;
        pStart += ret;
    }
    return true;
}

DBuffer DTCPSocket::SyncRecv(DUInt32 size, DUInt32* res)
{
    DByte* pBuf = (DByte*)malloc(size);
    if (pBuf == nullptr) {
        return DBuffer();
    }

    memset(pBuf, 0, size);
    DChar* pStart = (DChar*)pBuf;
    DUInt32 read = 0;
    while (read < size)
    {
        int ret = (int)recv(m_sock, pStart, size - read, 0);//MSG_DONTROUTE MSG_OOB
        if (ret == DSockError)
        {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReasonA = DNet::GetLastNetErrorStr(errCode);
            break;
        }
        else if (ret == 0)
        {
            break;
        }
        read += ret;
        pStart += ret;
    }

    DBuffer bufRet((DByte*)pBuf, read);
    free(pBuf);
    return bufRet;
}

////////////////////////////////////////////////////////////////////////////
// DTCPClient
// 要编写 IPv4 IPv6 都可以用的软件
// 1) 使用 SOCKADDR_STORAGE 替代 SOCKADDR_IN 和 SOCKADDR_IN6 
// 2) 不要使用 struct in_addr or struct in6_addr，还是用 SOCKADDR_STORAGE
// 3) 不要使用 hardcode addresses 

DTCPClient::DTCPClient()
{
    m_sock = DBadSocket;
    m_strRemoteIP.clear();
    m_wRemotePort = 0;
    m_pConnSink = nullptr;
    m_pDataSink = nullptr;
    m_nState = CONN_STATE_DISCONNECT;
}

DTCPClient::~DTCPClient()
{
    m_sock = DBadSocket;
    m_strRemoteIP.clear();
    m_wRemotePort = 0;
    m_pConnSink = nullptr;
    m_pDataSink = nullptr;
    m_nState = CONN_STATE_DISCONNECT;
}

DVoid DTCPClient::SetConnSink(DTCPClientSink* pSink)
{
    m_pConnSink = pSink;
}

DBool DTCPClient::Connect(std::string ip, DUInt16 wPort)
{
    if (m_nState != CONN_STATE_DISCONNECT) return false;

    m_strRemoteIP = ip;
    m_wRemotePort = wPort;
    m_nState = CONN_STATE_CONNECTING;
    DNet::AddConnReq(this, m_strRemoteIP, m_wRemotePort);
    return true;
}

DVoid DTCPClient::DisConnect()
{
    if (m_nState == CONN_STATE_DISCONNECT) return;

    m_nState = CONN_STATE_DISCONNECT;
    Shutdown(SD_BOTH);
    Close();

    m_strRemoteIP.clear();
    m_wRemotePort = 0;
}

DVoid DTCPClient::SetDataSink(DTCPDataSink* pSink)
{
    m_pDataSink = pSink;
}

DBool DTCPClient::Send(DBuffer buf)
{
    if (m_nState != CONN_STATE_CONNECTED) return false;

    DNet::AddSendReq(this, buf);
    return true;
}

DVoid* ReadThread(DVoid* pvParam)
{
    DTCPClient* pThis = (DTCPClient*)pvParam;
    if (pThis)
    {
        pThis->RecvLoop();
    }
    return 0;
}

DVoid DTCPClient::RecvLoop()
{
    DByte tempbuf[4096]; //max for 4K
    while (1)
    {
        DInt32 ret = (DInt32)recv(m_sock, (char*)tempbuf, 4096, 0); //MSG_DONTROUTE MSG_OOB
        if (ret == DSockError)
        {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReasonA = DNet::GetLastNetErrorStr(errCode);
            if (m_pDataSink)
            {
                m_pDataSink->OnBroken(this, errCode, strReasonA);
            }
            break;
        }
        else if (ret == 0)
        {
            if (m_pDataSink)
            {
                m_pDataSink->OnClose(this);
            }
            break;
        }
        else {
            DBuffer buf(tempbuf, ret);
            if (m_pDataSink)
            {
                m_pDataSink->OnRecvBuf(this, buf);
            }
        }
    }
}

DBool DTCPClient::StartRecv()
{
    if (!IsValid()) return false;

    m_recvthread = std::thread(ReadThread, this);
    return true;
}

DVoid DTCPClient::StopRecv()
{
    if (!IsValid()) return;
    
    Shutdown(SD_RECEIVE);
    m_recvthread.join();
}


