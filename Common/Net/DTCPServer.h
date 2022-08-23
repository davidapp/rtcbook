#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "DTCPClient.h"
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <utility>

class DTCPServerSink
{
public:
    virtual DVoid OnListening(DSocket sock, DUInt16 wPort) = 0;
    virtual DVoid OnListenOK(DSocket sock, DUInt16 wPort) = 0;
    virtual DVoid OnListenError(DSocket sock, DUInt32 code, std::string strReason) = 0;
    virtual DVoid OnNewConn(DSocket sock, DSocket newsock) = 0;
    virtual DVoid OnError(DSocket sock, DUInt32 code, std::string strReason) = 0;
    virtual DVoid OnStop(DSocket sock) = 0;

    virtual DVoid OnClose(DSocket sock) = 0;
    virtual DVoid OnBroken(DSocket sock, DUInt32 code, std::string strReason) = 0;

    virtual DVoid OnPreSend(DSocket sock, DBuffer buffer) = 0;
    virtual DVoid OnSendOK(DSocket sock) = 0;
    virtual DVoid OnSendError(DSocket sock, DUInt32 code, std::string strReason) = 0;
    virtual DVoid OnSendTimeout(DSocket sock) = 0;

    virtual DVoid OnRecvBuf(DSocket sock, DBuffer buf) = 0;

public:
    DTCPServerSink() { m_bIsAlive = true;  };
    virtual ~DTCPServerSink() { m_bIsAlive = false; }
    inline bool IsAlive() { return m_bIsAlive; }

private:
    DAtomBool m_bIsAlive;
};

#define SERVER_STATE_STOPED 0
#define SERVER_STATE_STARTING 1
#define SERVER_STATE_RUNNING 2

typedef struct tagDClientData {
    DSocket m_sock;
    std::string m_name;
    DBool m_bQuit;
} DClientData;

class DTCPServer : public DTCPSocket
{
public:
    DBool Start(DUInt16 wPort, DUInt16 backlog = 0);
    ~DTCPServer();

    virtual DVoid ServerLoop();
    virtual DVoid Stop();

public:
    DVoid       SetSink(DTCPServerSink* pSink);
    DUInt32     GetClientCount();
    DClientData GetClient(DInt32 index);
    DVoid       RemoveClient(DSocket client);

protected:
    DTCPServerSink* m_pListenSink;
    DTCPServerSink* m_pDataSink;
    DUInt16 m_wPort;
    DInt32  m_backlog;
    DAtomInt32  m_state;
    std::vector<DClientData> m_vecClients;
    std::mutex m_clientsMutex;
    std::shared_ptr<std::thread> m_serverthread;
};
