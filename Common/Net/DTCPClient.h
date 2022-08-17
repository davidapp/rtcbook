#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "Net/DTCP.h"
#include <string>
#include <thread>

class DTCPClient;

class DTCPClientSink
{
public:
    virtual DVoid OnConnecting(DTCPClient* sock, std::string strIP, DUInt16 wPort) = 0;
    virtual DVoid OnConnectOK(DTCPClient* sock) = 0;
    virtual DVoid OnConnectError(DTCPClient* sock, DUInt32 code, std::string strReason) = 0;
public:
    DTCPClientSink() {};
    virtual ~DTCPClientSink() {};
};

typedef struct tagDConnData
{
    DSocket sock;
    std::string strIP;
    DUInt16 wPort;
    DTCPClientSink* pSink;
} DConnData;


#define CONN_STATE_DISCONNECT 0
#define CONN_STATE_CONNECTING 1
#define CONN_STATE_CONNECTED 2

class DTCPClient : public DTCPSocket
{
public:
    DTCPClient();
    virtual ~DTCPClient();
    DVoid Init();
    DVoid UnInit();

public:
    // async connection methods
    DVoid SetConnSink(DTCPClientSink* pSink);
    DBool Connect(std::string strIP, DUInt16 wPort);
    DVoid DisConnect();
    DUInt32 GetState();
    static DVoid ConnThread(DVoid* pObj);
    DVoid ConnLoop();
    DTCPClientSink* m_pConnSink;
    std::string m_strRemoteIP;
    DUInt16 m_wRemotePort;
    DAtomInt32 m_nState;
    DVoid* m_connthread;

public:
    // async data methods
    DVoid SetDataSink(DTCPDataSink* pSink);
    DBool Send(DBuffer buf);
    DBool StartRecv();
    DVoid RecvLoop();
    DVoid StopRecv();
    DTCPDataSink* m_pDataSink;
    std::thread m_recvthread;

public:
    DHandle m_sendqueue;
    DVoid AddSendReq(DTCPClient* sock, DBuffer buffer);
};
