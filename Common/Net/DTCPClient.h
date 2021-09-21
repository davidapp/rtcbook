#pragma once

#include "DTypes.h"

class DTCPSocket
{
public:
	DTCPSocket();
	explicit DTCPSocket(DSocket sock);
	~DTCPSocket();

public:
	DBool Create(DBool bIPv6=false);
	DVoid Close();
	DVoid Attach(DSocket sock);
	DVoid Detach();
	DVoid Renew();

public:
	DSocket m_sock;
};


class DTCPClient : public DTCPSocket
{
public:
	DTCPClient();
	DTCPClient(DSocket sock, DCStr strIP, DUInt16 wPort);
	~DTCPClient();

public:
	// block method wrappers
	DBool SyncConnect(DCStr strIP, DUInt16 wPort);
	DBool SyncSend(DBuffer buf);
	DBuffer SyncRecv(DUInt32 size=1024);

public:
	// async connection methods
	DVoid SetConnSink(DTCPClientSink* pSink);
	DBool Connect(DStringA strIP, DUInt16 wPort);
	DVoid DisConnect();
	DTCPClientSink* m_pConnSink;
	DStringA m_strIP;
	DUInt16  m_wPort;

public:
	// async data methods
	DVoid SetDataSink(DTCPDataSink* pSink);
	DBool Send(DBuffer buf); // Post to Queue
	DBool Recv(); // Start a new thread to read
	DTCPDataSink* m_pDataSink;

public:
	DVoid BeginRead(); // Thread Function
	DUInt32 m_read;
};
