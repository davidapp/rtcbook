#pragma once

#include "DTypes.h"
#include <string>
#include "DTCPClient.h"

typedef struct tagDConnData
{
    DSocket sock;
    std::string strIP;
    DUInt16 wPort;
    DTCPClientSink* pSink;
} DConnData;

typedef struct tagDSendData
{
    DSocket sock;
    DByte* buffer;
    DTCPDataSink* pSink;
} DSendData;


class DNet
{
public:
    static DBool Init();
    static DBool UnInit();
    static DUInt32 GetLastNetError();
    static std::string GetLastNetErrorStr(DUInt32 errCode);
public:
    static DChar*  StrToUChar(DChar* pStr, DByte* pByte);
    static DUInt32 IPStrToUint32(DCStr strIP);

public:
    static DHandle GetConnQueue();
    static DHandle GetSendQueue();

    // TCP
    static DVoid AddConnReq(DTCPClient* sock, std::string strIP, DUInt16 wPort);
    static DVoid AddSendReq(DTCPClient* sock, DBuffer buffer);
    // UDP
    //static DVoid AddSendToReq(DUDPSocket* sock, std::string strIP, DUInt16 wPort, DBuffer buffer, DUDPDataSink* pSink);
};
