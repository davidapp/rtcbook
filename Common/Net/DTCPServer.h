#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "Base/DUtil.h"
#include "DTCP.h"
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <utility>

class DTCPServerSink
{
public:
    virtual DVoid OnListening(DSocket sock, DUInt16 wPort) {};
    virtual DVoid OnListenOK(DSocket sock, DUInt16 wPort) {};
    virtual DVoid OnListenError(DSocket sock, DUInt32 code, std::string strReason) {};
    virtual DVoid OnNewConn(DSocket sock, DSocket newsock) {};
    virtual DVoid OnError(DSocket sock, DUInt32 code, std::string strReason) {};
    virtual DVoid OnStop(DSocket sock) {};

    virtual DVoid OnClose(DSocket sock) {};
    virtual DVoid OnBroken(DSocket sock, DUInt32 code, std::string strReason) {};

    virtual DVoid OnPreSend(DSocket sock, DBuffer buffer) {};
    virtual DVoid OnSendOK(DSocket sock) {};
    virtual DVoid OnSendError(DSocket sock, DUInt32 code, std::string strReason) {};
    virtual DVoid OnSendTimeout(DSocket sock) {};

    virtual DVoid OnRecvBuf(DSocket sock, DBuffer buf) {};

public:
    DTCPServerSink() { m_bIsAlive = true;  };
    virtual ~DTCPServerSink() { m_bIsAlive = false; }
    inline bool IsAlive() { return m_bIsAlive; }

private:
    DAtomBool m_bIsAlive;
};

#define DTCPSERVER_STATE_STOPED 0
#define DTCPSERVER_STATE_STARTING 1
#define DTCPSERVER_STATE_RUNNING 2

typedef struct tagDClientData {
    DSocket m_sock;
    DUInt32 m_id;
    std::string m_name;
    DBool m_bQuit;
} DClientData;


class DTCPServer : public DTCPSocket
{
public:
    DTCPServer();
    ~DTCPServer();

public:
    DBool Start(DUInt16 wPort, DUInt16 backlog = 0);
    DBool Stop();
    DUInt16 m_wPort;
    DInt32  m_backlog;

    inline DUInt32 GetState() { return m_nObjState; }
    DAtomInt32 m_nObjState;
    std::string GetServerInfo();
    DSocket FindSockByID(DUInt32 id);
    DUInt32 FindIDBySock(DSocket sk);
    DBool SetIDName(DUInt32 id, std::string name);

public:
    DVoid SetSink(DTCPServerSink* pSink);
    DTCPServerSink* m_pRecvSink;
    DTCPServerSink* m_pSendSink;
    DRWLock m_SinkLock;

public:
    DVoid ReplyOne(DSocket sock, DBuffer buf);
    DVoid ReplyAll(DSocket sock, DBuffer buf);
    DUInt32 m_replyQueue;
    DVoid NotifyOtherNameChange(DSocket fromSock);

public:
    DUInt32     GetClientCount();
    DClientData GetClient(DUInt32 index);
    DVoid       RemoveClient(DSocket client);
    std::vector<DClientData> m_vecClients;
    std::mutex m_clientsMutex;
    DUInt32 m_gCounter;

protected:
    DVoid ServerLoop();
    std::shared_ptr<std::thread> m_serverthread;
    DSPinLock m_waitStart;
    DSPinLock m_waitFinish;
    DVoid Process(DBuffer buf, DSocket client);

private:
    DVoid SendOneEnterMsg(DSocket toSock, DUInt32 userID);
    DVoid SendOneLeaveMsg(DSocket toSock, DUInt32 userID);
    DVoid SendOnePeerMsg(DSocket toSock, DUInt32 fromID, std::string text);
    DVoid SendOneGroupMsg(DSocket toSock, std::string text);
};
