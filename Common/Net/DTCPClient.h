#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
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

class DTCPDataSink
{
public:
    virtual DVoid OnPreSend(DTCPClient* sock, DBuffer buffer) = 0;
    virtual DVoid OnSendOK(DTCPClient* sock) = 0;
    virtual DVoid OnSendError(DTCPClient* sock, DUInt32 code, std::string strReason) = 0;
    virtual DVoid OnSendTimeout(DTCPClient* sock) = 0;

    virtual DVoid OnRecvBuf(DTCPClient* sock, DBuffer buf) = 0;
    virtual DVoid OnClose(DTCPClient* sock) = 0;
    virtual DVoid OnBroken(DTCPClient* sock, DUInt32 code, std::string strReason) = 0;

public:
    DTCPDataSink() {};
    virtual ~DTCPDataSink() {};
};


class DTCPSocket
{
public:
    DTCPSocket();
    explicit DTCPSocket(DSocket sock);
    ~DTCPSocket();

public:
    DBool Create(DBool bIPv6 = false);
    DVoid Close();
    DVoid Attach(DSocket sock);
    DVoid Detach();
    DVoid Renew();
    DBool operator==(const DTCPSocket sock);
    DBool IsValid();

public:
    DBool Bind(DUInt16 port);
    DBool Listen(DInt32 backlog);
    DTCPSocket Accept();
    DInt32 Shutdown(DInt32 how);

public:
    // block method wrappers
    DBool SyncConnect(DCStr strIP, DUInt16 wPort);
    DBool SyncSend(DBuffer buf);
    DBuffer SyncRecv(DUInt32 size, DUInt32 *res);

public:
    DSocket m_sock;
};


#define CONN_STATE_DISCONNECT 0
#define CONN_STATE_CONNECTING 1
#define CONN_STATE_CONNECTED 2

class DTCPClient : public DTCPSocket
{
public:
    DTCPClient();
    ~DTCPClient();

public:
    // async connection methods
    DVoid SetConnSink(DTCPClientSink* pSink);
    DBool Connect(std::string strIP, DUInt16 wPort);
    DVoid DisConnect();
    DTCPClientSink* m_pConnSink;
    std::string m_strRemoteIP;
    DUInt16 m_wRemotePort;
    DAtomInt32 m_nState;

public:
    // async data methods
    DVoid SetDataSink(DTCPDataSink* pSink);
    DBool Send(DBuffer buf);
    DBool StartRecv();
    DVoid RecvLoop();
    DVoid StopRecv();
    DTCPDataSink* m_pDataSink;
    std::thread m_recvthread;
};
