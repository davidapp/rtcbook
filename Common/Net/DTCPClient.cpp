#include "DTCPClient.h"
#include "Base/DUtil.h"
#include "Base/DMsgQueue.h"
#include "Net/DNet.h"
#include <assert.h>
#include <memory>

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

#define DM_SET_SINK  1001
#define DM_NET_CONN  1002
#define DM_NET_SEND  1003


DTCPClient::DTCPClient()
{
    m_sock = DBadSocket;
    m_strRemoteIP.clear();
    m_wRemotePort = 0;
    m_workqueue = 0;
    m_pSendSink = nullptr;
    m_pRecvSink = nullptr;
    m_nObjState = DTCPCLIENT_STATE_UNINIT;
    m_refCount = 0;
}

DTCPClient::~DTCPClient()
{
    UnInit();
    m_sock = DBadSocket;
    m_strRemoteIP.clear();
    m_wRemotePort = 0;
    m_workqueue = 0;
    m_pSendSink = nullptr;
    m_pRecvSink = nullptr;
    m_nObjState = DTCPCLIENT_STATE_UNINIT;
}

DVoid* DX86_STDCALL WorkHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    if (msg == DM_SET_SINK) {
        DTCPClient* pThis = (DTCPClient*)para1;
        pThis->m_pSendSink = (DTCPClientSink*)para2;
    }
    else if (msg == DM_NET_CONN)
    {
        DTCPClient* pThis = (DTCPClient*)para1;
        if (!pThis->Create())
        {
            if (pThis->m_pSendSink && pThis->m_pSendSink->IsAlive()) {
                DUInt32 errCode = DNet::GetLastNetError();
                std::string strReason = DNet::GetLastNetErrorStr(errCode);
                pThis->m_pSendSink->OnConnectError(pThis->m_sock, errCode, strReason);
            }
            return NULL;
        }

#if defined(BUILD_FOR_WINDOWS)
        SOCKADDR_IN addr;
        memset(&addr, 0, sizeof(SOCKADDR_IN));
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, pThis->m_strRemoteIP.c_str(), &addr.sin_addr.s_addr);
        addr.sin_port = htons(pThis->m_wRemotePort);
#else
        sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, pThis->m_strRemoteIP.c_str(), &addr.sin_addr);
        addr.sin_port = htons(pThis->m_wRemotePort);
#endif
        pThis->m_nObjState = DTCPCLIENT_STATE_CONNECTING;
        if (pThis->m_pSendSink && pThis->m_pSendSink->IsAlive())
        {
            pThis->m_pSendSink->OnConnecting(pThis->m_sock, pThis->m_strRemoteIP, pThis->m_wRemotePort);
        }
#if defined(BUILD_FOR_WINDOWS)
        int ret = connect(pThis->m_sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
#else
        int ret = connect(pThis->m_sock, (sockaddr*)&addr, sizeof(sockaddr_in));
#endif
        if (ret == DSockError)
        {
            pThis->m_nObjState = DTCPCLIENT_STATE_DISCONNECT;

            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            if (pThis->m_pSendSink && pThis->m_pSendSink->IsAlive())
            {
                pThis->m_pSendSink->OnConnectError(pThis->m_sock, errCode, strReason);
            }

            pThis->m_waitConnFinish.Signal();
        }
        else {
            pThis->m_nObjState = DTCPCLIENT_STATE_CONNECTED;

            if (pThis->m_pSendSink && pThis->m_pSendSink->IsAlive())
            {
                pThis->m_pSendSink->OnConnectOK(pThis->m_sock);
            }

            // 启动 Recv 线程
            pThis->StartRecv();
        }
    }
    else if (msg == DM_NET_SEND)
    {
        DTCPClient* pThis = (DTCPClient*)para1;
        DBuffer buf;
        buf.Attach((DByte*)para2);

        if (pThis->m_pSendSink && pThis->m_pSendSink->IsAlive())
        {
            pThis->m_pSendSink->OnPreSend(pThis->m_sock, buf);
        }
        DUInt32 sent = 0;
        DUInt32 size = buf.GetSize();
        while (sent < size)
        {
            DChar* pStart = (DChar*)buf.GetBuf();
            DInt32 ret = (DInt32)send(pThis->m_sock, pStart, size - sent, 0); //MSG_DONTROUTE MSG_OOB
            if (ret == DSockError)
            {
                DUInt32 errCode = DNet::GetLastNetError();
                std::string strReasonA = DNet::GetLastNetErrorStr(errCode);
                if (pThis->m_pSendSink && pThis->m_pSendSink->IsAlive())
                {
                    pThis->m_pSendSink->OnSendError(pThis->m_sock, errCode, strReasonA);
                }
                return NULL;
            }
            sent += ret;
            pStart += ret;
        }
        if (pThis->m_pSendSink && pThis->m_pSendSink->IsAlive())
        {
            pThis->m_pSendSink->OnSendOK(pThis->m_sock);
        }
    }
    return NULL;
}

DVoid* DX86_STDCALL WorkCleaner(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    if (msg == DM_NET_SEND)
    {
        DBuffer buf;
        buf.Attach((DByte*)para2);
    }
    return NULL;
}

DVoid DTCPClient::Init()
{
    if (m_nObjState == DTCPCLIENT_STATE_UNINIT) {
        m_workqueue = DMsgQueue::Create("WorkQueue", 100);
        DMsgQueue::AddHandler(m_workqueue, WorkHandler);
        DMsgQueue::SetCleaner(m_workqueue, WorkCleaner);
        m_nObjState = DTCPCLIENT_STATE_DISCONNECT;
    }
}

DVoid DTCPClient::UnInit()
{
    m_pSendSink = nullptr;
    m_pRecvSink = nullptr;

    DisConnect();

    if (m_nObjState == DTCPCLIENT_STATE_DISCONNECT) {
        DMsgQueue::PostQuitMsg(m_workqueue);
        if (IsValid()) {
            m_waitRecvFinish.Reset();
            Close();
            DInt32 res = m_waitRecvFinish.Wait(200);
            assert(res >= 0);
        }
        DMsgQueue::RemoveQueue(m_workqueue);
        m_nObjState = DTCPCLIENT_STATE_UNINIT;
    }
}

DBool DTCPClient::SetSink(DTCPClientSink* pSink)
{
    if (m_nObjState == DTCPCLIENT_STATE_UNINIT || m_nObjState == DTCPCLIENT_STATE_CONNECTING) {
        return false;
    }
    // 对于 m_pSendSink 使用队列修改
    DMsgQueue::PostQueueMsg(m_workqueue, DM_SET_SINK, this, pSink);

    m_SinkLock.LockWrite();
    m_pRecvSink = pSink;
    m_SinkLock.UnlockWrite();

    return true;
}


DBool DTCPClient::Connect(std::string ip, DUInt16 wPort, DUInt32 timeout_ms)
{
    if (m_nObjState != DTCPCLIENT_STATE_DISCONNECT) return false;

    m_strRemoteIP = ip;
    m_wRemotePort = wPort;

    DMsgQueue::PostQueueMsg(m_workqueue, DM_NET_CONN, this, (DVoid*)(DUInt64)timeout_ms);
    return true;
}

DVoid DTCPClient::DisConnect()
{
    if (m_nObjState == DTCPCLIENT_STATE_DISCONNECT) return;

    if (m_nObjState == DTCPCLIENT_STATE_CONNECTING) {
        m_nObjState = DTCPCLIENT_STATE_DISCONNECT;
        m_waitConnFinish.Reset();
        Close();
        DUInt32 res = m_waitConnFinish.Wait(200);
        assert(res > 0);
    }
    else if (m_nObjState == DTCPCLIENT_STATE_CONNECTED) {
        m_nObjState = DTCPCLIENT_STATE_DISCONNECT;
        m_waitRecvFinish.Reset();
#if defined(BUILD_FOR_WINDOWS)
        Shutdown(SD_SEND); // 优雅的关闭
#else
        Shutdown(SHUT_WR); // 优雅的关闭
#endif
        DUInt32 res = m_waitRecvFinish.Wait(200);
        if (res == 0) {
            // 如果 Server 长时间不响应，我们也不傻等了，线程必须退出
            m_waitRecvFinish.Reset();
            Close();
            res = m_waitRecvFinish.Wait(200);
            assert(res > 0);
        }
    }
}


DBool DTCPClient::Send(DBuffer buf)
{
    if (m_nObjState != DTCPCLIENT_STATE_CONNECTED) return false;

    DMsgQueue::PostQueueMsg(m_workqueue, DM_NET_SEND, this, buf.GetBuf());
    buf.Detach();

    return true;
}

DBool DTCPClient::StartRecv()
{
    if (!IsValid()) return false;

    m_recvthread.reset(new std::thread(&DTCPClient::RecvLoop, this));
    return true;
}

DVoid DTCPClient::RecvLoop()
{
    DInt32 ret = 0;
    DByte tempbuf[16]; //max for 4K
    while (1)
    {
        ret = (DInt32)recv(m_sock, (char*)tempbuf, 4, 0); //MSG_DONTROUTE MSG_OOB
        if (ret == DSockError)
        {
            m_nObjState = DTCPCLIENT_STATE_DISCONNECT;
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReasonA = DNet::GetLastNetErrorStr(errCode);
            m_SinkLock.LockRead();
            if (m_pRecvSink && m_pRecvSink->IsAlive())
            {
                m_pRecvSink->OnBroken(m_sock, errCode, strReasonA);
            }
            m_SinkLock.UnlockRead();
            break;
        }
        else if (ret == 0)
        {
            m_nObjState = DTCPCLIENT_STATE_DISCONNECT;
            m_SinkLock.LockRead();
            if (m_pRecvSink && m_pRecvSink->IsAlive())
            {
                m_pRecvSink->OnClose(m_sock);
            }
            m_SinkLock.UnlockRead();
            break;
        }
        else {
            DBuffer buf(tempbuf, 4);
            DReadBuffer rb(buf);
            DUInt32 nSize = rb.ReadUInt32(true);
            DTCPSocket sock(m_sock);
            DBuffer bufContent = sock.SyncRecv(nSize, &ret);
            m_SinkLock.LockRead();
            if (m_pRecvSink && m_pRecvSink->IsAlive())
            {
                m_pRecvSink->OnRecvBuf(m_sock, bufContent);
            }
            m_SinkLock.UnlockRead();
        }
    }
    if (ret <= 0) {
        Close();
        m_recvthread->detach();
        m_waitRecvFinish.Signal();
    }
}
