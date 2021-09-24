#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"

class DWinSocket
{
public:
    DWinSocket();
	~DWinSocket();

public:
    DVoid Create(HWND hWnd);
	DVoid Close();
	DVoid Detach();
	DVoid Renew();

public:
    DBool Connect(DCStr strIP, DUInt16 wPort);
    DBool Send(DBuffer buf);
    DBuffer Recv(DUInt32 size = 1024);

public:
	DSocket m_sock;
    std::string m_strIP;
    DUInt16  m_wPort;
};
