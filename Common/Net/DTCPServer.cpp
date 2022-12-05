#include "DTCPServer.h"
#include "Base/DXP.h"
#include "Base/DMsgQueue.h"
#include "Net/DNet.h"

#define SERVER_SEND_MSG 10000
#define SERVER_SENDALL_MSG 10001
DVoid* DX86_STDCALL SendHandler(DUInt32 msg, DVoid* para1, DVoid* para2);
DVoid* DX86_STDCALL SendCleaner(DUInt32 msg, DVoid* para1, DVoid* para2);

#define HELLO_SC_CMD_CNAME  101
#define HELLO_SC_CMD_LEAVE  102
#define HELLO_SC_CMD_PMSG   103
#define HELLO_SC_CMD_GMSG   104


DTCPServer::DTCPServer()
{
    m_nObjState = DTCPSERVER_STATE_STOPED;
    m_wPort = 0;
    m_backlog = 0;
    m_pRecvSink = nullptr;
    m_pSendSink = nullptr;
    m_vecClients.clear();
    m_sendQueue = 0;
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
    m_sendQueue = 0;
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

    // 创建 Server 监听线程
    m_waitStart.Reset();
    m_serverthread.reset(new std::thread(&DTCPServer::ServerLoop, this));
    m_waitStart.Wait(300);
    // 创建发送队列
    m_sendQueue = DMsgQueue::Create("ServerSend");
    DMsgQueue::AddHandler(m_sendQueue, SendHandler);
    DMsgQueue::SetCleaner(m_sendQueue, SendCleaner);
    m_nObjState = DTCPSERVER_STATE_STARTING;
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
        if (res > 0) {
            return false;
        }
    }

    DMsgQueue::Quit(m_sendQueue);
    m_nObjState = DTCPSERVER_STATE_STOPED;
    return true;
}

DVoid DTCPServer::ServerLoop()
{
    m_waitStart.Signal();

    if (!Create())
    {
        if (m_pRecvSink && m_pRecvSink->IsAlive()) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pRecvSink->OnListenError(this->m_sock, errCode, strReason);
        }
        m_nObjState = DTCPSERVER_STATE_STOPED;
        return;
    }

    if (!Bind(m_wPort)) {
        if (m_pRecvSink && m_pRecvSink->IsAlive()) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pRecvSink->OnListenError(this->m_sock, errCode, strReason);
        }
        m_nObjState = DTCPSERVER_STATE_STOPED;
        return;
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
        m_nObjState = DTCPSERVER_STATE_STOPED;
        return;
    }

    m_nObjState = DTCPSERVER_STATE_RUNNING;
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

        if (ret == DSockError)
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
                cdata.m_id = m_gCounter++;
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
                        // 读取 4 字节包头长度
                        DInt32 res = 0;
                        DBuffer buf = client.SyncRecv(4, &res);
                        if (res == 0) {
                            // 客户端主动退出
                            if (m_pRecvSink && m_pRecvSink->IsAlive()) {
                                m_pRecvSink->OnClose((*item).m_sock);
                            }
                            // 广播退出消息
                            SendOneLeaveMsg(FindIDBySock((*item).m_sock));
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
                            // 广播退出消息
                            SendOneLeaveMsg(FindIDBySock((*item).m_sock));
                            (*item).m_bQuit = true;
                            client.Close();
                        }
                        client.Detach();
                    }
                }
                // 清理掉退出的 ClientData 
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
}

DVoid DTCPServer::SetSink(DTCPServerSink* pSink)
{
    m_pRecvSink = pSink;
    m_pSendSink = pSink;
}

std::string DTCPServer::GetServerInfo()
{
    std::string strRet;
    strRet += "Server State: ";
    strRet += DXP::UInt32ToStr(m_nObjState);
    strRet += D_LINES;

    strRet += "Server Port: ";
    strRet += DXP::UInt16ToStr(m_wPort);
    strRet += D_LINES;

    DUInt32 nCount = GetClientCount();
    strRet += "Server Users: ";
    strRet += DXP::UInt32ToStr(nCount);
    strRet += D_LINES;

    for (DUInt32 i = 0; i < nCount; i++)
    {
        DClientData data = GetClient(i);
        strRet += "ID: ";
        strRet += DXP::UInt32ToStr(data.m_id);
        strRet += "  Name: ";
        strRet += data.m_name;
        strRet += D_LINES;
    }

    return strRet;
}

DSocket DTCPServer::FindSockByID(DUInt32 id)
{
    DUInt32 nCount = GetClientCount();
    DSocket sockRet = 0;
    for (DUInt32 i = 0; i < nCount; i++)
    {
        DClientData data = GetClient(i);
        if (data.m_id == id) {
            sockRet = data.m_sock;
            break;
        }
    }
    return sockRet;
}

DUInt32 DTCPServer::FindIDBySock(DSocket sk)
{
    DUInt32 nCount = GetClientCount();
    DUInt32 IDRet = 0;
    for (DUInt32 i = 0; i < nCount; i++)
    {
        DClientData data = GetClient(i);
        if (data.m_sock == sk) {
            IDRet = data.m_id;
            break;
        }
    }
    return IDRet;
}

DBool DTCPServer::SetIDName(DUInt32 id, std::string name)
{
    DUInt32 nCount = GetClientCount();
    DBool bFind = false;
    for (DUInt32 i = 0; i < nCount; i++)
    {
        DClientData data = GetClient(i);
        if (data.m_id == id) {
            m_clientsMutex.lock();
            m_vecClients[i].m_name = name;
            m_clientsMutex.unlock();
            bFind = true;
            break;
        }
    }
    if (bFind) return true;
    return false;
}


DVoid DTCPServer::AsyncSend(DSocket sock, DBuffer buf)
{
    DMsgQueue::PostQueueMsg(m_sendQueue, SERVER_SEND_MSG, buf.GetBuf(), (DVoid*)sock);
    buf.Detach();
}

DVoid DTCPServer::NotifyNameChange(DSocket fromSock, std::string newName)
{
    DUInt32 fromID = FindIDBySock(fromSock);
    m_clientsMutex.lock();
    for (DUInt32 i = 0; i < m_vecClients.size(); i++)
    {
        SendOneCNameMsg(m_vecClients[i].m_sock, fromID, newName);
    }
    m_clientsMutex.unlock();
}

DUInt32 DTCPServer::GetClientCount()
{
    m_clientsMutex.lock();
    DUInt32 cSize = m_vecClients.size();
    m_clientsMutex.unlock();
    return cSize;
}

DClientData DTCPServer::GetClient(DUInt32 index)
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


DVoid* DX86_STDCALL SendHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    if (msg == SERVER_SEND_MSG)
    {
        DBuffer buf;
        buf.Attach((DByte*)para1);
        DSocket sock = (DSocket)para2;
        DTCPSocket ts(sock);
        ts.SyncSend(buf);
    }
    return nullptr;
}

DVoid* DX86_STDCALL SendCleaner(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    if (msg == SERVER_SEND_MSG)
    {
        DBuffer buf;
        buf.Attach((DByte*)para1);
    }
    return nullptr;
}

DVoid DTCPServer::SendOneCNameMsg(DSocket toSock, DUInt32 userID, std::string name)
{
    DGrowBuffer gb;
    gb.AddUInt32(1 + 4 + 2 + name.size(), true);
    gb.AddUInt8(HELLO_SC_CMD_CNAME);
    gb.AddUInt32(userID, true);
    DBuffer bufName((DByte*)name.c_str(), name.size());
    gb.AddUInt16((DUInt16)name.size(), true);
    gb.AddFixBuffer(bufName);
    DBuffer bufSend = gb.Finish();
    AsyncSend(toSock, bufSend);
    bufSend.Detach();
}

DVoid DTCPServer::SendOneLeaveMsg(DUInt32 userID)
{
    DGrowBuffer gb;
    gb.AddUInt32(1 + 4, true);
    gb.AddUInt8(HELLO_SC_CMD_LEAVE);
    gb.AddUInt32(userID, true);
    DBuffer bufSend = gb.Finish();
    m_clientsMutex.lock();
    for (DUInt32 i = 0; i < m_vecClients.size(); i++)
    {
        if (m_vecClients[i].m_id != userID)
        {
            AsyncSend(m_vecClients[i].m_sock, bufSend);
        }
    }
    m_clientsMutex.unlock();
    bufSend.Detach();
}

DVoid DTCPServer::SendOneMsg(DSocket toSock, DUInt32 fromID, std::string text)
{
    DGrowBuffer gb;
    gb.AddUInt32(1 + 4 + 4 + text.size(), true);
    gb.AddUInt8(HELLO_SC_CMD_PMSG);
    gb.AddUInt32(fromID, true);
    gb.AddStringA(text);
    DBuffer bufSend = gb.Finish();
    AsyncSend(toSock, bufSend);
    bufSend.Detach();
}

DVoid DTCPServer::SendGroupMsg(DSocket toSock, std::string text)
{
    DUInt32 fromID = FindIDBySock(toSock);
    DGrowBuffer gb;
    gb.AddUInt32(1 + 4 + 4 + text.size(), true);
    gb.AddUInt8(HELLO_SC_CMD_GMSG);
    gb.AddUInt32(fromID, true);
    gb.AddStringA(text);
    DBuffer bufSend = gb.Finish();
    m_clientsMutex.lock();
    for (DUInt32 i = 0; i < m_vecClients.size(); i++)
    {
        AsyncSend(m_vecClients[i].m_sock, bufSend);
    }
    m_clientsMutex.unlock();
    bufSend.Detach();
}
