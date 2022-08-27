﻿#include "DTCPServer.h"
#include "Base/DUtil.h"
#include "Base/DMsgQueue.h"
#include "Net/DNet.h"
#include <assert.h>

DVoid* DX86_STDCALL ReplyHandler(DUInt32 msg, DVoid* para1, DVoid* para2);


DTCPServer::DTCPServer()
{
    m_nObjState = DTCPSERVER_STATE_STOPED;
    m_wPort = 0;
    m_backlog = 0;
    m_pRecvSink = nullptr;
    m_pSendSink = nullptr;
    m_vecClients.clear();
    m_replyQueue = 0;
}

DTCPServer::~DTCPServer()
{
    Stop();
    m_nObjState = DTCPSERVER_STATE_STOPED;
    m_wPort = 0;
    m_backlog = 0;
    m_pRecvSink = nullptr;
    m_pSendSink = nullptr;
    m_vecClients.clear();
    m_replyQueue = 0;
}

DBool DTCPServer::Start(DUInt16 wPort, DUInt16 backlog)
{
    if (m_nObjState != DTCPSERVER_STATE_STOPED) {
        return false;
    }

    m_wPort = wPort;
    m_backlog = backlog;
    m_clientsMutex.lock();
    m_vecClients.clear();
    m_clientsMutex.unlock();
    m_serverthread.reset(new std::thread(&DTCPServer::ServerLoop, this));
    m_nObjState = DTCPSERVER_STATE_STARTING;

    m_replyQueue = DMsgQueue::Create("ServerReply");
    DMsgQueue::AddHandler(m_replyQueue, ReplyHandler);
    return true;
}

DBool DTCPServer::Stop()
{
    if (m_nObjState == DTCPSERVER_STATE_STOPED) {
        return false;
    }

    if (IsValid()) {
        m_waitFinish.Reset();
        Close();
        DInt32 res = m_waitFinish.Wait(200);
        assert(res > 0);
    }

    DMsgQueue::PostQuitMsg(m_replyQueue);
    DMsgQueue::RemoveQueue(m_replyQueue);
    m_nObjState = DTCPSERVER_STATE_STOPED;
    return true;
}

DVoid DTCPServer::ServerLoop()
{
    if (!Create())
    {
        if (m_pRecvSink && m_pRecvSink->IsAlive()) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pRecvSink->OnListenError(this->m_sock, errCode, strReason);
        }
        return;
    }

    if (!Bind(m_wPort)) {
        if (m_pRecvSink && m_pRecvSink->IsAlive()) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pRecvSink->OnListenError(this->m_sock, errCode, strReason);
        }
    }
    else {
        if (m_pRecvSink && m_pRecvSink->IsAlive()) {
            m_pRecvSink->OnListening(this->m_sock, m_wPort);
        }
    }

    if (Listen(m_backlog)) {
        if (m_pRecvSink && m_pRecvSink->IsAlive()) {
            m_pRecvSink->OnListenOK(this->m_sock, m_wPort);
        }
    }
    else {
        if (m_pRecvSink && m_pRecvSink->IsAlive()) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pRecvSink->OnListenError(this->m_sock, errCode, strReason);
        }
    }

    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexp;
    timeval select_timeout = { 10, 0 };
    DInt32 ret = 0;
    while (TRUE)
    {
        if (!IsValid()) {
            if (m_pRecvSink && m_pRecvSink->IsAlive()) {
                m_pRecvSink->OnStop(this->m_sock);
            }
            break;
        }

        FD_ZERO(&fdread);
        FD_SET(m_sock, &fdread);

        for (auto item = m_vecClients.begin(); item != m_vecClients.end(); item++) {
            FD_SET((*item).m_sock, &fdread);
        }

        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexp);
        ret = select(0, &fdread, NULL, NULL, &select_timeout);

        if (ret == SOCKET_ERROR)
        {
            if (m_pRecvSink && m_pRecvSink->IsAlive()) {
                DUInt32 errCode = DNet::GetLastNetError();
                std::string strReason = DNet::GetLastNetErrorStr(errCode);
                m_pRecvSink->OnError(this->m_sock, errCode, strReason);
            }
            break;
        }
        else if (ret >= 0)
        {
            if (FD_ISSET(m_sock, &fdread))
            {
                if (!IsValid()) {
                    if (m_pRecvSink && m_pRecvSink->IsAlive()) {
                        m_pRecvSink->OnStop(this->m_sock);
                    }
                    break;
                }

                // 3.If listen has been called and a connection is pending, the accept function will succeed.
                DTCPSocket client = Accept();
                DClientData cdata;
                cdata.m_sock = client.m_sock;
                cdata.m_name = client.GetName();
                cdata.m_bQuit = false;
                m_clientsMutex.lock();
                m_vecClients.push_back(cdata);
                m_clientsMutex.unlock();
                if (m_pRecvSink && m_pRecvSink->IsAlive()) {
                    m_pRecvSink->OnNewConn(this->m_sock, client.m_sock);
                }
            }
            else {
                // 1.Data is available for reading.
                // 2.Connection has been closed, reset, or terminated.
                for (auto item = m_vecClients.begin(); item != m_vecClients.end(); item++) {
                    if (FD_ISSET((*item).m_sock, &fdread))
                    {
                        DTCPSocket client;
                        client.Attach((*item).m_sock);
                        DInt32 res = 0;
                        DBuffer buf = client.SyncRecv(4, &res);
                        if (res == 0) {
                            // 客户端主动退出
                            if (m_pRecvSink && m_pRecvSink->IsAlive()) {
                                m_pRecvSink->OnClose((*item).m_sock);
                            }
                            // TODO 广播一下退出消息
                            (*item).m_bQuit = true;
                            client.Shutdown(SD_BOTH);
                            client.Close();
                        }
                        else if (res > 0) {
                            // 正常收到 Buffer
                            DReadBuffer rb(buf);
                            DUInt32 bufLength = rb.ReadUInt32(true);
                            DBuffer bufContent = client.SyncRecv(bufLength, &ret);
                            if (res > 0) {
                                Process(bufContent, (*item).m_sock);
                            }
                        }
                        else if (res < 0) {
                            // 客户端出了问题
                            if (m_pRecvSink && m_pRecvSink->IsAlive()) {
                                DUInt32 errCode = DNet::GetLastNetError();
                                std::string strReason = DNet::GetLastNetErrorStr(errCode);
                                m_pRecvSink->OnBroken((*item).m_sock, errCode, strReason);
                            }
                            // TODO 广播一下异常消息
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

    m_serverthread->detach();
    m_waitFinish.Signal();
}

DVoid DTCPServer::Process(DBuffer buf, DSocket client)
{
    if (m_pSendSink && m_pSendSink->IsAlive()) {
        m_pSendSink->OnRecvBuf(client, buf);
    }

    DReadBuffer rbContent(buf);
    DUInt8 cmd = rbContent.ReadUInt8();
    // 看一下命令字
    if (cmd == 1) {
        std::wstring wstr = rbContent.ReadString();
        // 给房间内所有其他 socket 发送一条消息


        //DMsgQueue::PostQueueMsg(m_replyQueue, SERVER_REPLY_MSG_RECV, buf, (DVoid*)this);
    }
    else if (cmd == 2) {

    }
}

DVoid DTCPServer::SetSink(DTCPServerSink* pSink)
{
    m_pRecvSink = pSink;
    m_pSendSink = pSink;
}

DUInt32 DTCPServer::GetClientCount()
{
    m_clientsMutex.lock();
    DUInt32 cSize = m_vecClients.size();
    m_clientsMutex.unlock();
    return cSize;
}

DClientData DTCPServer::GetClient(DInt32 index)
{
    m_clientsMutex.lock();
    DClientData cData = m_vecClients[index];
    m_clientsMutex.unlock();
    return cData;
}

DVoid DTCPServer::RemoveClient(DSocket client)
{
    m_clientsMutex.lock();
    for (auto item = m_vecClients.begin(); item != m_vecClients.end(); item++) {
        if ((*item).m_sock == client) {
            m_vecClients.erase(item);
            break;
        }
    }
    m_clientsMutex.unlock();
}


DVoid* DX86_STDCALL ReplyHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
{

    return nullptr;
}