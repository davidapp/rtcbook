#include "DTCPClient.h"
#include "Base/DUtil.h"
#include "Base/DMsgQueue.h"
#include "Net/DNet.h"

#if defined(BUILD_FOR_WINDOWS)
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h>     //IPPROTO_TCP
#include <unistd.h>
#include <string.h>
#endif

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
    m_nObjState = CONN_STATE_DISCONNECT;
}

DTCPClient::~DTCPClient()
{
    m_sock = DBadSocket;
    m_strRemoteIP.clear();
    m_wRemotePort = 0;
    m_pConnSink = nullptr;
    m_pDataSink = nullptr;
    m_nObjState = CONN_STATE_DISCONNECT;
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
            pSink->OnPreSend(sock->m_sock, buf);
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
                    pSink->OnSendError(sock->m_sock, errCode, strReasonA);
                }
                delete pData;
                return NULL;
            }
            sent += ret;
            pStart += ret;
        }
        if (pSink)
        {
            pSink->OnSendOK(sock->m_sock);
        }
        delete pData;
    }
    return NULL;
}

DVoid DTCPClient::Init()
{
    m_sendqueue = DMsgQueue::Create("SendQueue", 100);
    DMsgQueue::AddHandler(m_sendqueue, SendHandler);
}

DVoid DTCPClient::UnInit()
{
    DMsgQueue::PostQuitMsg(m_sendqueue);
    if (IsValid()) {
        Close();
    }
    DMsgQueue::RemoveQueue(m_sendqueue);
}

DVoid DTCPClient::SetConnSink(DTCPClientSink* pSink)
{
    m_pConnSink = pSink;
}

DVoid DTCPClient::ConnLoop()
{
    if (!Create())
    {
        if (m_pConnSink) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pConnSink->OnConnectError(this, errCode, strReason);
        }
        return;
    }

#if defined(BUILD_FOR_WINDOWS)
    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(SOCKADDR_IN));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, m_strRemoteIP.c_str(), &addr.sin_addr.s_addr);
    addr.sin_port = htons(m_wRemotePort);
#else
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, pData->strIP, &addr.sin_addr);
    addr.sin_port = htons(pData->wPort);
#endif
    m_nObjState = CONN_STATE_CONNECTING;
    if (m_pConnSink)
    {
        m_pConnSink->OnConnecting(this, m_strRemoteIP, m_wRemotePort);
    }
#if defined(BUILD_FOR_WINDOWS)
    if (connect(m_sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
#else
    if (connect(m_sock, (sockaddr*)&addr, sizeof(sockaddr_in)) == DSockError)
#endif
    {
        DUInt32 errCode = DNet::GetLastNetError();
        std::string strReason = DNet::GetLastNetErrorStr(errCode);
        if (m_pConnSink)
        {
            m_nObjState = CONN_STATE_DISCONNECT;
            m_pConnSink->OnConnectError(this, errCode, strReason);
        }
    }
    else {
        if (m_pConnSink)
        {
            m_nObjState = CONN_STATE_CONNECTED;
            m_pConnSink->OnConnectOK(this);
        }
    }
    m_connThread->detach();
}

DBool DTCPClient::Connect(std::string ip, DUInt16 wPort)
{
    if (m_nObjState != CONN_STATE_DISCONNECT) return false;

    m_strRemoteIP = ip;
    m_wRemotePort = wPort;
    m_connThread.reset(new std::thread(&DTCPClient::ConnLoop, this));
    return true;
}

DVoid DTCPClient::DisConnect()
{
    if (m_nObjState == CONN_STATE_DISCONNECT) return;

    m_nObjState = CONN_STATE_DISCONNECT;
    Shutdown(SD_BOTH);
    Close();
}

DUInt32 DTCPClient::GetState()
{
    return m_nObjState;
}

DVoid DTCPClient::SetDataSink(DTCPDataSink* pSink)
{
    m_pDataSink = pSink;
}

DBool DTCPClient::Send(DBuffer buf)
{
    if (m_nObjState != CONN_STATE_CONNECTED) return false;

    AddSendReq(this, buf);
    return true;
}

DVoid DTCPClient::RecvLoop()
{
    DInt32 ret = 0;
    DByte tempbuf[4096]; //max for 4K
    while (1)
    {
        ret = (DInt32)recv(m_sock, (char*)tempbuf, 4096, 0); //MSG_DONTROUTE MSG_OOB
        if (ret == DSockError)
        {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReasonA = DNet::GetLastNetErrorStr(errCode);
            if (m_pDataSink)
            {
                m_pDataSink->OnBroken(m_sock, errCode, strReasonA);
            }
            break;
        }
        else if (ret == 0)
        {
            if (m_pDataSink)
            {
                m_pDataSink->OnClose(m_sock);
            }
            break;
        }
        else {
            DBuffer buf(tempbuf, ret);
            if (m_pDataSink)
            {
                m_pDataSink->OnRecvBuf(m_sock, buf);
            }
        }
    }
    if (ret == 0) {
        DTCPSocket sock(m_sock);
        sock.Close();
    }
}

DBool DTCPClient::StartRecv()
{
    if (!IsValid()) return false;

    m_recvthread.reset(new std::thread(&DTCPClient::RecvLoop, this));
    return true;
}

DVoid DTCPClient::StopRecv()
{
    if (!IsValid()) return;
    
    Shutdown(SD_RECEIVE);
    m_recvthread->join();
}

DVoid DTCPClient::AddSendReq(DTCPClient* sock, DBuffer buffer)
{
    DSendData* pData = new DSendData();
    pData->sock = sock->m_sock;
    pData->buffer = buffer.GetBuf();
    pData->pSink = sock->m_pDataSink;
    buffer.Detach();
    DMsgQueue::PostQueueMsg(m_sendqueue, DM_NET_SEND, pData, sock);
}
