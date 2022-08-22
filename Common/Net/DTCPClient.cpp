#include "DTCPClient.h"
#include "Base/DUtil.h"
#include "Base/DMsgQueue.h"
#include "Net/DNet.h"
#include <assert.h>

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

#define DM_SET_VALUE  1001
#define DM_CALL_BACK  1002
#define DM_NET_SEND   1003


DTCPClient::DTCPClient()
{
    m_sock = DBadSocket;
    m_strRemoteIP.clear();
    m_wRemotePort = 0;
    m_workqueue = 0;
    m_pConnSink = nullptr;
    m_pDataSink = nullptr;
    m_nObjState = CONN_STATE_DISCONNECT;
}

DTCPClient::~DTCPClient()
{
    m_sock = DBadSocket;
    m_strRemoteIP.clear();
    m_wRemotePort = 0;
    m_workqueue = 0;
    m_pConnSink = nullptr;
    m_pDataSink = nullptr;
    m_nObjState = CONN_STATE_DISCONNECT;
}

DVoid* DX86_STDCALL WorkHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
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
    m_workqueue = DMsgQueue::Create("WorkQueue", 100);
    DMsgQueue::AddHandler(m_workqueue, WorkHandler);
}

DVoid DTCPClient::UnInit()
{
    DMsgQueue::PostQuitMsg(m_workqueue);
    if (IsValid()) {
        Close();
    }
    DMsgQueue::RemoveQueue(m_workqueue);
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
    int ret = connect(m_sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
#else
    int ret = connect(m_sock, (sockaddr*)&addr, sizeof(sockaddr_in));
#endif
    if (ret == DSockError)
    {
        m_nObjState = CONN_STATE_DISCONNECT;

        DUInt32 errCode = DNet::GetLastNetError();
        std::string strReason = DNet::GetLastNetErrorStr(errCode);
        if (m_pConnSink)
        {
            m_pConnSink->OnConnectError(this, errCode, strReason);
        }
    }
    else {
        m_nObjState = CONN_STATE_CONNECTED;

        if (m_pConnSink)
        {
            m_pConnSink->OnConnectOK(this);
        }

        // 启动 Recv 线程
        StartRecv();
    }
    m_connThread->detach();
    m_wait.Signal();
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

    if (m_nObjState == CONN_STATE_CONNECTING) {
        m_nObjState = CONN_STATE_DISCONNECT;
        m_wait.Reset();
        Close();
        DUInt32 res = m_wait.Wait(200);
        assert(res > 0);
    }
    else if (m_nObjState == CONN_STATE_CONNECTED) {
        m_nObjState = CONN_STATE_DISCONNECT;
        m_wait.Reset();
        Shutdown(SD_SEND); // 优雅的关闭
        DUInt32 res = m_wait.Wait(200);
        if (res == 0) {
            // 如果 Server 长时间不响应，我们也不傻等了，线程必须退出
            m_wait.Reset();
            Close();
            res = m_wait.Wait(200);
            assert(res > 0);
        }
    }
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
            m_nObjState = CONN_STATE_DISCONNECT;
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
            m_nObjState = CONN_STATE_DISCONNECT;
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
    if (ret <= 0) {
        Close();
        m_recvthread->detach();
        m_wait.Signal();
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
    DMsgQueue::PostQueueMsg(m_workqueue, DM_NET_SEND, pData, sock);
}
