#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include <string>

class DTCPDataSink
{
public:
    virtual DVoid OnPreSend(DSocket sock, DBuffer buffer) = 0;
    virtual DVoid OnSendOK(DSocket sock) = 0;
    virtual DVoid OnSendError(DSocket sock, DUInt32 code, std::string strReason) = 0;
    virtual DVoid OnSendTimeout(DSocket sock) = 0;

    virtual DVoid OnRecvBuf(DSocket sock, DBuffer buf) = 0;
    virtual DVoid OnClose(DSocket sock) = 0;
    virtual DVoid OnBroken(DSocket sock, DUInt32 code, std::string strReason) = 0;

public:
    DTCPDataSink() {};
    virtual ~DTCPDataSink() {};
};


typedef struct tagDSendData
{
    DSocket sock;
    DByte* buffer;
    DTCPDataSink* pSink;
} DSendData;

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
    std::string GetName();

public:
    DBool Bind(DUInt16 port);
    DBool Listen(DInt32 backlog);
    DTCPSocket Accept();
    DInt32 Shutdown(DInt32 how);

public:
    // block method wrappers
    DBool SyncConnect(DCStr strIP, DUInt16 wPort);
    DBool SyncSend(DBuffer buf);
    DBuffer SyncRecv(DUInt32 size, DUInt32* res);

public:
    DSocket m_sock;
};
