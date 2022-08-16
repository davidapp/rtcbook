#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "DTCPClient.h"
#include <string>
#include <vector>
#include <mutex>

class DTCPServer;

class DTCPServerSink
{
public:
    virtual DVoid OnListening(DTCPServer* sock, DUInt16 wPort) = 0;
    virtual DVoid OnListenOK(DTCPServer* sock, DUInt16 wPort) = 0;
    virtual DVoid OnListenError(DTCPServer* sock, DUInt32 code, std::string strReason) = 0;
    virtual DVoid OnListenStop(DTCPServer* sock, DUInt32 code, std::string strReason) = 0;
    virtual DVoid OnNewConn(DTCPServer* sock, DTCPClient* newsock) = 0;
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
    DTCPClient* GetClient(DInt32 index);
    DVoid       RemoveClient(DTCPClient* client);

protected:
    DTCPServerSink* m_pListenSink;
    DTCPDataSink* m_pDataSink;
    DUInt16 m_wPort;
    DInt32  m_backlog;
    DInt32  m_state;
    std::vector<DTCPSocket> m_vecClients;
    std::mutex m_clientsMutex;

private:
    static DUInt32 ServerThread(DVoid* pThis);
};
