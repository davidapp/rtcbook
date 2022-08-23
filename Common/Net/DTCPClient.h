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
    virtual DVoid OnConnecting(DSocket sock, std::string strIP, DUInt16 wPort) {};
    virtual DVoid OnConnectOK(DSocket sock) {};
    virtual DVoid OnConnectError(DSocket sock, DUInt32 code, std::string strReason) {};

    virtual DVoid OnClose(DSocket sock) {};
    virtual DVoid OnBroken(DSocket sock, DUInt32 code, std::string strReason) {};

    virtual DVoid OnPreSend(DSocket sock, DBuffer buffer) {};
    virtual DVoid OnSendOK(DSocket sock) {};
    virtual DVoid OnSendError(DSocket sock, DUInt32 code, std::string strReason) {};
    virtual DVoid OnSendTimeout(DSocket sock) {};

    virtual DVoid OnRecvBuf(DSocket sock, DBuffer buf) {};

public:
    DTCPClientSink() { m_bIsAlive = true; }
    virtual ~DTCPClientSink() { m_bIsAlive = false; }
    inline bool IsAlive() { return m_bIsAlive; }

private:
    DAtomBool m_bIsAlive;
};


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
    DVoid SetSink(DTCPClientSink* pSink);
    DTCPClientSink* m_pSendSink;
    DTCPClientSink* m_pRecvSink;
    DRWLock m_SinkLock;

    DBool Connect(std::string strIP, DUInt16 wPort, DUInt32 timeout_ms = 5000);
    DVoid DisConnect();
    std::string m_strRemoteIP;
    DUInt16 m_wRemotePort;
    DSPinLock m_wait;

public:
    // async data methods
    DBool Send(DBuffer buf);
    DHandle m_workqueue;

    // async recv
    DBool StartRecv();
    DVoid StopRecv();
    DVoid RecvLoop();
    std::shared_ptr<std::thread> m_recvthread;
};
