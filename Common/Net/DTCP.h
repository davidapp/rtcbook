#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include <string>


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
    DInt32 GetFlag();
    DInt32 SetFlag(DInt32 newFlag);
    DVoid SetNonBlock();
    DVoid SetBlock();

public:
    // block method wrappers
    DBool SyncConnect(DCStr strIP, DUInt16 wPort);
    DBool SyncSend(DBuffer buf);
    DBuffer SyncRecv(DUInt32 size, DInt32* res);

public:
    DSocket m_sock;
};
