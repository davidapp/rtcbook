#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "DTCPClient.h"
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <utility>

class DTCPServer;

class DTCPServerSink
{
public:
    virtual DVoid OnListening(DTCPServer* sock, DUInt16 wPort) = 0;
    virtual DVoid OnListenOK(DTCPServer* sock, DUInt16 wPort) = 0;
    virtual DVoid OnListenError(DTCPServer* sock, DUInt32 code, std::string strReason) = 0;
    virtual DVoid OnNewConn(DTCPServer* sock, DTCPSocket newsock) = 0;
    virtual DVoid OnError(DTCPServer* sock, DUInt32 code, std::string strReason) = 0;
    virtual DVoid OnStop(DTCPServer* sock) = 0;
public:
    DTCPServerSink() {};
    virtual ~DTCPServerSink() {};
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
    DVoid       SetListenSink(DTCPServerSink* pSink);
    DVoid       SetDataSink(DTCPDataSink* pSink);
    DUInt32     GetClientCount();
    DClientData GetClient(DInt32 index);
    DVoid       RemoveClient(DSocket client);

protected:
    DTCPServerSink* m_pListenSink;
    DTCPDataSink* m_pDataSink;
    DUInt16 m_wPort;
    DInt32  m_backlog;
    DAtomInt32  m_state;
    std::vector<DClientData> m_vecClients;
    std::mutex m_clientsMutex;
    std::shared_ptr<std::thread> m_serverthread;
};
