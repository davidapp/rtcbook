#include "DTCP.h"
#include "DNet.h"
#include "Base/DXP.h"

#if defined(BUILD_FOR_WINDOWS)
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h>     //IPPROTO_TCP
#include <unistd.h>
#include <string.h>
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

DTCPSocket::DTCPSocket(const DTCPSocket& sock)
{
    m_sock = sock.m_sock;
}

DTCPSocket& DTCPSocket::operator=(const DTCPSocket& sock)
{
    m_sock = sock.m_sock;
    return *this;
}

DBool DTCPSocket::Create()
{
    if (m_sock == DBadSocket)
    {
        m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_sock == DBadSocket)
        {
            return false;
        }
    }
    return true;
}

DVoid DTCPSocket::Close()
{
    if (m_sock != DBadSocket)
    {
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
        closesocket(m_sock);
#else
        close(m_sock);
#endif
        m_sock = DBadSocket;
    }
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

std::string DTCPSocket::GetName()
{
    sockaddr_in sa = {0};
    int sa_size = sizeof(sa);
#if defined(BUILD_FOR_WINDOWS)
    getpeername(m_sock, (sockaddr*)&sa, &sa_size);
#else
    getpeername(m_sock, (sockaddr*)&sa, (socklen_t*)&sa_size);
#endif
    char ip[100] = { 0 };
#if defined(BUILD_FOR_WINDOWS)
    inet_ntop(AF_INET, &sa.sin_addr.S_un.S_addr, ip, 100);
#else
    inet_ntop(AF_INET, &sa.sin_addr, ip, 100);
#endif
    std::string ret = ip;
    ret += ":";
    ret += DXP::UInt16ToStr(sa.sin_port);
    return ret;
}

DBool DTCPSocket::Bind(DUInt16 port)
{
#if defined(BUILD_FOR_WINDOWS)
    SOCKADDR_IN InternetAddr = {};
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(port);
    DInt32 ret = bind(m_sock, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr));
#else
    sockaddr_in InternetAddr = {};
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(port);
    DInt32 ret = bind(m_sock, (const struct sockaddr *)&InternetAddr, sizeof(InternetAddr));
#endif
    if (ret != 0) {
        return false;
    }
    return true;
}

DBool DTCPSocket::Listen(DInt32 backlog)
{
    DInt32 ret = listen(m_sock, backlog);
    if (ret != 0) {
        return false;
    }
    return true;
}

DTCPSocket DTCPSocket::Accept()
{
    DTCPSocket sockRet;
    DSocket NewConnection = accept(m_sock, 0, 0);
    sockRet.Attach(NewConnection);
    return sockRet;
}

DInt32 DTCPSocket::Shutdown(DInt32 how)
{
    return shutdown(m_sock, how);
}

DBool DTCPSocket::SetNonBlock()
{
#if defined(BUILD_FOR_WINDOWS)
    u_long ul = 1;
    DInt32 nRet = ioctlsocket(m_sock, FIONBIO, &ul);
#else
    int flags = fcntl(m_sock, F_GETFL, 0);
    DInt32 nRet = fcntl(m_sock, F_SETFL, flags | O_NONBLOCK);
#endif
    if (nRet == DSockError)
    {
        return false;
    }
    return true;
}

DBool DTCPSocket::SetBlock()
{
#if defined(BUILD_FOR_WINDOWS)
    u_long ul = 0;
    DInt32 nRet = ioctlsocket(m_sock, FIONBIO, &ul);
#else
    int flags = fcntl(m_sock, F_GETFL, 0);
    DInt32 nRet = fcntl(m_sock, F_SETFL, flags & ~O_NONBLOCK);
#endif
    if (nRet == DSockError)
    {
        return false;
    }
    return true;
}

DUInt32 DTCPSocket::GetBufRead()
{
#if defined(BUILD_FOR_WINDOWS)
    u_long ul = 0;
    DInt32 nRet = ioctlsocket(m_sock, FIONREAD, &ul);
    if (nRet == DSockError)
    {
        return 0;
    }
    return (DUInt32)ul;
#else
    return 0;
#endif
}

DInt32 DTCPSocket::IsListen()
{
    DInt32 optVal;
    DInt32 optLen = sizeof(DInt32);
#if defined(BUILD_FOR_WINDOWS)
    DInt32 ret = getsockopt(m_sock, SOL_SOCKET, SO_ACCEPTCONN, (char*)&optVal, &optLen);
#else
    DInt32 ret = getsockopt(m_sock, SOL_SOCKET, SO_ACCEPTCONN, (void*)&optVal, (socklen_t *)&optLen);
#endif
    if (ret == DSockError) {
        return 0;
    }
    return optVal;
}

DBool DTCPSocket::SyncConnect(DCStr strIP, DUInt16 wPort)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(SOCKADDR_IN));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = DNet::IPStrToUint32(strIP);
    addr.sin_port = DXP::Swap16(wPort);
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
        std::string strReasonA = DNet::GetLastNetErrorStr(errCode);
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

DBuffer DTCPSocket::SyncRecv(DUInt32 size, DInt32* res)
{
    DByte* pBuf = (DByte*)malloc(size);
    if (pBuf == nullptr) {
        return DBuffer();
    }

    memset(pBuf, 0, size);
    DChar* pStart = (DChar*)pBuf;
    DUInt32 read = 0;
    int ret = 0;
    while (read < size)
    {
        ret = (int)recv(m_sock, pStart, size - read, 0);//MSG_DONTROUTE MSG_OOB
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
    *res = ret;

    DBuffer bufRet((DByte*)pBuf, read);
    free(pBuf);
    return bufRet;
}
