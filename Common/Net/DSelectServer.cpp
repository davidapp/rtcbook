#include "DSelectServer.h"
#include "DNet.h"
#include "Base/DMsgQueue.h"
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <vector>

DVoid* DX86_STDCALL ReplyHandler(DUInt32 msg, DVoid* para1, DVoid* para2);

DVoid DSelectServer::ServerLoop()
{
    if (!Create())
    {
        if (m_pListenSink) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pListenSink->OnListenError(this, errCode, strReason);
        }
        return;
    }

    if (!Bind(m_wPort)) {
        if (m_pListenSink) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pListenSink->OnListenError(this, errCode, strReason);
        }
    }
    else {
        if (m_pListenSink) {
            m_pListenSink->OnListening(this, m_wPort);
        }
    }

    if (Listen(m_backlog)) {
        if (m_pListenSink) {
            m_pListenSink->OnListenOK(this, m_wPort);
        }

        m_replyQueue = DMsgQueue::Create("SelectReply", 100);
        DMsgQueue::AddHandler(m_replyQueue, ReplyHandler);
    }
    else {
        if (m_pListenSink) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pListenSink->OnListenError(this, errCode, strReason);
        }
    }

    fd_set fdread;
    // 1.Data is available for reading.
    // 2.Connection has been closed, reset, or terminated.
    // 3.If listen has been called and a connection is pending, the accept function will succeed.
    fd_set fdwrite;
    fd_set fdexp;
    timeval select_timeout = { 10, 0 };
    int ret = 0;
    while (TRUE)
    {
        if (!IsValid()) {
            break;
        }

        FD_ZERO(&fdread);
        FD_SET(m_sock, &fdread);

        for (auto item = m_vecClients.begin(); item != m_vecClients.end(); item++) {
            FD_SET((*item).m_sock, &fdread);
        }

        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexp);

        if ((ret = select(0, &fdread, NULL, NULL, &select_timeout)) == SOCKET_ERROR)
        {
            if (m_pListenSink) {
                DUInt32 errCode = DNet::GetLastNetError();
                std::string strReason = DNet::GetLastNetErrorStr(errCode);
                m_pListenSink->OnError(this, errCode, strReason);
            }
            Close();
            break;
        }

        if (ret > 0)
        {
            if (FD_ISSET(m_sock, &fdread))
            {
                if (!IsValid()) {
                    break;
                }
                DTCPSocket client = Accept();
                m_vecClients.push_back(client);
                if (m_pListenSink) {
                    m_pListenSink->OnNewConn(this, client);
                }
            }
            else {
                for (auto item = m_vecClients.begin(); item != m_vecClients.end(); item++) {
                    if (FD_ISSET((*item).m_sock, &fdread))
                    {
                        DTCPSocket client;
                        client.Attach((*item).m_sock);
                        DUInt32 res;
                        DBuffer buf = client.SyncRecv(4, &res);
                        DReadBuffer rb(buf);
                        DUInt32 bufLength = rb.ReadUInt32(true);
                        DBuffer bufContent = client.SyncRecv(bufLength, &res);
                        DMsgQueue::PostQueueMsg(m_replyQueue, 1, bufContent.GetBuf(), (DVoid*)(*item).m_sock);
                        bufContent.Detach();
                        client.Detach();
                    }
                }
            }
        }
    }
    
    DMsgQueue::PostQuitMsg(m_replyQueue);

    if (m_pListenSink) {
        m_pListenSink->OnStop(this);
    }
}

DVoid DSelectServer::Stop()
{
    Close(); // Close ª·»√ select ∑µªÿ
    DMsgQueue::RemoveQueue(m_replyQueue);
}

DVoid* DX86_STDCALL ReplyHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    DBuffer bufRecv;
    bufRecv.Attach((DByte*)para1);
    DReadBuffer rb(bufRecv);

    DTCPSocket client;
    client.Attach((DSocket)para2);
    if (msg == 1) {

    }
    return nullptr;
}