#include "DWinSocket.h"

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


////////////////////////////////////////////////////////////////////////////
// DTCPClient
// 要编写 IPv4 IPv6 都可以用的软件
// 1) 使用 SOCKADDR_STORAGE 替代 SOCKADDR_IN 和 SOCKADDR_IN6 
// 2) 不要使用 struct in_addr or struct in6_addr，还是用 SOCKADDR_STORAGE
// 3) 不要使用 hardcode addresses 

DTCPClient::DTCPClient()
{
    m_sock = DBadSocket;
    m_strIP.Empty();
    m_wPort = 0;
    m_pConnSink = NULL;
    m_pDataSink = NULL;
    m_read = 0;
    Create();
}

DTCPClient::DTCPClient(DSocket sock, DCStr strIP, DUInt16 wPort)
{
    m_sock = sock;
    m_strIP = strIP;
    m_wPort = wPort;
    m_pConnSink = NULL;
    m_pDataSink = NULL;
    m_read = 0;
}

DTCPClient::~DTCPClient()
{
    Close();

    m_strIP.Empty();
    if (m_read != 0)
    {
        DThread::WaitFinish(m_read, 5);
    }
    m_read = 0;	//The thread must be dead before dealloc!
}

DBool DTCPClient::SyncConnect(DCStr strIP, DUInt16 wPort)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(SOCKADDR_IN));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = DIP::IPStrToUint32(strIP);
    addr.sin_port = DNet::Swap16(wPort);
    if (connect(m_sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        DUInt32 errCode = DError::GetLastNetError();
        DStringA strReasonA = DError::GetLastNetErrorStr();
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
        DUInt32 errCode = DError::GetLastNetError();
        D_UNUSED(errCode);
        DStringA strReasonA = DError::GetLastNetErrorStr();
        return false;
    }
#endif
    return true;
}

DBool DTCPClient::SyncSend(DBuffer buf)
{
    DUInt32 sent = 0;
    DChar* pStart = (DChar*)buf.GetBuf();
    DUInt32 sizeAll = buf.GetSize();
    while (sent < sizeAll)
    {
        DInt32 ret = (DInt32)send(m_sock, pStart, sizeAll - sent, 0);//MSG_DONTROUTE MSG_OOB
        if (ret == DSockError)
        {
            DUInt32 errCode = DError::GetLastNetError();
            D_UNUSED(errCode);
            DStringA strReasonA = DError::GetLastNetErrorStr();
            return false;
        }
        sent += ret;
        pStart += ret;
    }
    return true;
}

DBuffer DTCPClient::SyncRecv(DUInt32 size)
{
    DByte* pBuf = (DByte*)DALLOC(size);
    DMemZero(pBuf, size);
    DChar* pStart = (DChar*)pBuf;
    DUInt32 read = 0;
    while (read < size)
    {
        int ret = (int)recv(m_sock, pStart, size - read, 0);//MSG_DONTROUTE MSG_OOB
        if (ret == DSockError)
        {
            DUInt32 errCode = DError::GetLastNetError();
            D_UNUSED(errCode);
            DStringA strReasonA = DError::GetLastNetErrorStr();
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
    DFREE(pBuf);
    return bufRet;
}

DVoid DTCPClient::SetConnSink(DTCPClientSink* pSink)
{
    m_pConnSink = pSink;
}

DBool DTCPClient::Connect(DStringA ip, DUInt16 wPort)
{
    DNetIO::AddConnReq(this, ip, wPort);
    m_strIP = ip;
    m_wPort = wPort;
    return true;
}

DVoid DTCPClient::DisConnect()
{
    shutdown(m_sock, 2);//SD_BOTH
    Close();
    m_strIP.Empty();
}

DVoid DTCPClient::SetDataSink(DTCPDataSink* pSink)
{
    m_pDataSink = pSink;
}

DVoid* ReadThread(DVoid* pvParam)
{
    DTCPClient* pThis = (DTCPClient*)pvParam;
    if (pThis)
    {
        pThis->BeginRead();
    }
    return 0;
}

DVoid DTCPClient::BeginRead()
{
    DByte tempbuf[4096]; //max for 4K
    while (1)
    {
        DInt32 ret = (DInt32)recv(m_sock, (char*)tempbuf, 4096, 0);//MSG_DONTROUTE MSG_OOB
        if (ret == DSockError)
        {
            DUInt32 errCode = DError::GetLastNetError();
            DStringA strReasonA = DError::GetLastNetErrorStr();
            if (m_pDataSink)
            {
                m_pDataSink->OnBroken(this, errCode, strReasonA);
            }
            DLogDev(logFilter, "[%d] return as (ret==-1).", m_read);

            if (m_read)
            {
                //DLog("[%d] Remove Thread Info.\n", m_read);
                DThread::CloseThreadHandle(m_read);
                m_read = 0;
            }

            //we assume it is a Server Client if m_pConnSink == NULL
            if (m_pConnSink == NULL)
            {
                /*if (m_pServer)
                {
                    m_pServer->RemoveClient(this);
                }*/
            }
            else
            {
                this->m_strIP.Empty();
                Renew();	//we renew the client broken socket
            }
            return;
        }
        else if (ret == 0)
        {
            if (m_pDataSink)
            {
                m_pDataSink->OnClose(this);
            }
            DLogDev(logFilter, "[%d] return as (ret==0).", m_read);

            if (m_read)
            {
                DLogDev(logFilter, "[%d] Remove Thread Info.", m_read);
                DThread::CloseThreadHandle(m_read);
                m_read = 0;//We set it to zero to prevent the dealloc wait for the thread!
            }

            //we assume it is a Server Client if m_pConnSink == NULL
            if (m_pConnSink == NULL)
            {
                /*if (m_pServer)
                {
                    m_pServer->RemoveClient(this);
                }*/
            }
            else
            {
                this->m_strIP.Empty();
                Renew();	//we renew the client broken socket
            }
            return;
        }

        DBuffer buf(tempbuf, ret);
        if (m_pDataSink)
        {
            m_pDataSink->OnRecvBuf(this, buf);
        }
    }
}

DBool DTCPClient::Recv()
{
    if (m_read == 0)
    {
        if (m_sock)
        {
            DBool bResult = DThread::Create(ReadThread, this, &m_read);
            if (m_pConnSink == NULL)
            {
                DLogDev(logFilter, "Server Recv Thread %d Begin.", m_read);
            }
            else
            {
                DLogDev(logFilter, "Client Recv Thread %d Begin.", m_read);
            }
            return bResult;
        }
        return false;
    }
    else
    {
        return true;
    }
}

DBool DTCPClient::Send(DBuffer buf)
{
    if (m_sock)
    {
        //DNetIO::AddSendReq(this, buf);
    }
    return true;
}
