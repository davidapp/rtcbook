#include "DTCP.h"
#include "DNet.h"
#include "Base/DUtil.h"

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

std::string DTCPSocket::GetName()
{
    sockaddr_in sa = {0};
    int sa_size = sizeof(sa);
    getpeername(m_sock, (sockaddr*)&sa, &sa_size);
    char ip[100] = { 0 };
    char port[10] = { 0 };
    inet_ntop(AF_INET, &sa.sin_addr.S_un.S_addr, ip, 100);
    std::string ret = ip;
    ret += ":";
    _itoa_s(sa.sin_port, port, 10);
    ret += port;
    return ret;
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

DInt32 DTCPSocket::GetFlag()
{
    return 0;// fcntl(m_sock, F_GETFL, 0);
}

DInt32 DTCPSocket::SetFlag(DInt32 newFlag)
{
    return 0;
}

DBool DTCPSocket::SetNonBlock()
{
    unsigned long ul = 1;
    int nRet = ioctlsocket(m_sock, FIONBIO, (unsigned long*)&ul);
    if (nRet == SOCKET_ERROR)
    {
        return false;
    }
    return true;
}

DBool DTCPSocket::SetBlock()
{
    unsigned long ul = 0;
    int nRet = ioctlsocket(m_sock, FIONBIO, (unsigned long*)&ul);
    if (nRet == SOCKET_ERROR)
    {
        return false;
    }
    return true;
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
