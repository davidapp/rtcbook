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


class DTCPServer : public DTCPSocket
{
public:
    DBool Start(DUInt16 wPort, DUInt16 backlog = 0);

    virtual DVoid ServerLoop();
    virtual DVoid Stop();

public:
    DVoid       SetListenSink(DTCPServerSink* pSink);
    DVoid       SetDataSink(DTCPDataSink* pSink);
    DUInt32     GetClientCount();
    DTCPSocket  GetClient(DInt32 index);
    DVoid       RemoveClient(DTCPSocket client);

protected:
    DTCPServerSink* m_pListenSink;
    DTCPDataSink* m_pDataSink;
    DUInt16 m_wPort;
    DInt32  m_backlog;
    DInt32  m_state;
    std::vector<DTCPSocket> m_vecClients;
    std::mutex m_clientsMutex;
    std::shared_ptr<std::thread> m_serverthread;
};
