#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "Base/DUtil.h"
#include "Net/DTCP.h"
#include <string>
#include <thread>
#include <memory>

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
    DAtomInt32 m_nObjState;
    inline DUInt32 GetState() { return m_nObjState; }

public:
    // async connection methods
    DVoid SetConnSink(DTCPClientSink* pSink);
    DTCPClientSink* m_pConnSink;

    DBool Connect(std::string strIP, DUInt16 wPort);
    DVoid DisConnect();
    DVoid ConnLoop();
    std::string m_strRemoteIP;
    DUInt16 m_wRemotePort;
    std::shared_ptr<std::thread> m_connThread;
    DSPinLock m_wait;

public:
    // async data methods
    DVoid SetDataSink(DTCPDataSink* pSink);
    DTCPDataSink* m_pDataSink;

    DBool Send(DBuffer buf);
    DVoid AddSendReq(DTCPClient* sock, DBuffer buffer);
    DHandle m_workqueue;

    // async recv
    DBool StartRecv();
    DVoid StopRecv();
    DVoid RecvLoop();
    std::shared_ptr<std::thread> m_recvthread;
};
