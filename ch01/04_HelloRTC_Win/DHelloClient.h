#pragma once

#include "Net/DTCPClient.h"
#include "Protocol/DHelloProtocol.h"
#include <map>

class DHelloClient
{
public:
    static DVoid SendHeartBeat(DTCPClient* pClient)
    {
        DGrowBuffer gb;
        gb.AddUInt32(1, true);
        gb.AddUInt8(HELLO_CS_CMD_HEARTBEAT);
        DBuffer buf = gb.Finish();
        pClient->Send(buf);
    }

    static DVoid SendGetInfo(DTCPClient* pClient)
    {
        DGrowBuffer gb;
        gb.AddUInt32(1, true);
        gb.AddUInt8(HELLO_CS_CMD_GETINFO);
        DBuffer buf = gb.Finish();
        pClient->Send(buf);
    }

    static DVoid SendIDText(DTCPClient* pClient, DUInt32 id, std::string text)
    {
        DGrowBuffer gb;
        gb.AddUInt32(1, true);
        gb.AddUInt8(HELLO_CS_CMD_SENDTEXT);
        gb.AddUInt32(id, true);
        gb.AddStringA(text);
        DBuffer buf = gb.Finish();
        pClient->Send(buf);
    }

    static DVoid SendSetName(DTCPClient* pClient, std::string name)
    {
        DGrowBuffer gb;
        gb.AddUInt32(1, true);
        gb.AddUInt8(HELLO_CS_CMD_SETNAME);
        gb.AddUInt16((DUInt16)name.size(), true);
        DBuffer bufname((DByte*)name.c_str(), name.size());
        gb.AddFixBuffer(bufname);
        DBuffer buf = gb.Finish();
        pClient->Send(buf);
    }

    static DVoid HandleRecvBuffer(HWND hWnd, DSocket sock, DBuffer recvBuf)
    {
        DReadBuffer rb(recvBuf);
        DUInt8 cmd = rb.ReadUInt8();
        if (cmd == HELLO_CS_CMD_HEARTBEAT)
        {

        }
        else if (cmd == HELLO_CS_CMD_GETINFO)
        {

        }
        else if (cmd == HELLO_CS_CMD_SENDTEXT)
        {

        }
        else if (cmd == HELLO_CS_CMD_SETNAME)
        {

        }
        else if (cmd == HELLO_CS_CMD_BROADCAST)
        {

        }
        else if (cmd == HELLO_CS_CMD_PUSH)
        {

        }
    }

    static DInt32 FindIDByName(std::string name)
    {
        if (users.find(name) != users.end())
        {
            return users[name];
        }
        return -1;
    }

    static std::map<std::string, DUInt32> users;
};
