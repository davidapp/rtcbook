﻿#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"


class DTCPSocket
{
public:
    DTCPSocket();
    explicit DTCPSocket(DSocket sock);
    ~DTCPSocket();
    DTCPSocket(const DTCPSocket& sock);
    DTCPSocket& operator=(const DTCPSocket& sock);

public:
    DBool Create();
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
    DBool SetNonBlock();
    DBool SetBlock();
    DUInt32 GetBufRead();
    DInt32 IsListen();

public:
    // block method wrappers
    DBool SyncConnect(DCStr strIP, DUInt16 wPort);
    DBool SyncSend(DBuffer buf);
    DBuffer SyncRecv(DUInt32 size, DInt32* res);

public:
    DSocket m_sock;
};
