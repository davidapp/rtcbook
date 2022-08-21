#include "DSelectServer.h"
#include "DNet.h"
#include "Base/DMsgQueue.h"
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <vector>

#define SERVER_REPLY_MSG_INIT 1
#define SERVER_REPLY_MSG_RECV 2


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
                DClientData cdata;
                cdata.m_sock = client.m_sock;
                cdata.m_name = client.GetName();
                cdata.m_bQuit = false;
                m_vecClients.push_back(cdata);
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
                        DInt32 res;
                        DBuffer buf = client.SyncRecv(4, &res);
                        if (res == 0) {
                            // �ͻ��������˳�
                            if (m_pDataSink) {
                                m_pDataSink->OnClose((*item).m_sock);
                            }
                            // TODO �㲥һ���˳���Ϣ
                            (*item).m_bQuit = true;
                            client.Shutdown(SD_BOTH);
                            client.Close();
                        }
                        else if (res > 0) {
                            DReadBuffer rb(buf);
                            DUInt32 bufLength = rb.ReadUInt32(true);
                            DBuffer bufContent = client.SyncRecv(bufLength, &res);
                            if (res > 0) {
                                Process(bufContent, (*item).m_sock);
                            }
                        }
                        else if (res < 0) {
                            // �ͻ��˳�������
                            if (m_pDataSink) {
                                DUInt32 errCode = DNet::GetLastNetError();
                                std::string strReason = DNet::GetLastNetErrorStr(errCode);
                                m_pDataSink->OnBroken((*item).m_sock, errCode, strReason);
                            }
                            // TODO �㲥һ���쳣��Ϣ
                            (*item).m_bQuit = true;
                            client.Close();
                        }
                        client.Detach();
                    }
                }
                int last = m_vecClients.size() - 1;
                for (int i = last; i >= 0; i--)
                {
                    if (m_vecClients[i].m_bQuit) {
                        m_vecClients.erase(std::begin(m_vecClients) + i);
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

// �� client �յ� buf �Ĵ���
DVoid DSelectServer::Process(DBuffer buf, DSocket client)
{
    if (m_pDataSink) {
        m_pDataSink->OnRecvBuf(client, buf);
    }

    DReadBuffer rbContent(buf);
    DUInt8 cmd = rbContent.ReadUInt8();
    // ��һ��������
    if (cmd == 1) {
        std::wstring wstr = rbContent.ReadString();
        // ���������������� socket ����һ����Ϣ


        //DMsgQueue::PostQueueMsg(m_replyQueue, SERVER_REPLY_MSG_RECV, buf, (DVoid*)this);
    }
    else if (cmd == 2) {

    }
}


DVoid DSelectServer::Stop()
{
    Close(); // Close ���� select ����
    DMsgQueue::RemoveQueue(m_replyQueue);
}

DVoid* DX86_STDCALL ReplyHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
{

    return nullptr;
}