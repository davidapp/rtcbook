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

    static DVoid HandleRecvBuffer(HWND hWnd, DSocket sock, DBuffer recvBuf, std::map<std::string, DUInt32>& data)
    {
        DReadBuffer rb(recvBuf);
        DUInt8 cmd = rb.ReadUInt8();
        if (cmd == HELLO_CS_CMD_HEARTBEAT)
        {
            //IGNORED
        }
        else if (cmd == HELLO_CS_CMD_GETINFO)
        {
            DUInt8 res = rb.ReadUInt8();
            if (res == HELLO_RESULT_SUCCESS)
            {
                data.clear();
                DUInt16 nCount = rb.ReadUInt16(true);
                for (DUInt16 i = 0; i < nCount; i++)
                {
                    DUInt32 nID = rb.ReadUInt32(true);
                    DUInt16 nNameSize = rb.ReadUInt16(true);
                    DBuffer bufName = rb.ReadFixBuffer(nNameSize);
                    std::string strName((DChar*)bufName.GetBuf(), bufName.GetSize());
                    data.insert(std::make_pair(strName, nID));
                }
            }
        }
        else if (cmd == HELLO_CS_CMD_SENDTEXT)
        {
            //IGNORED
        }
        else if (cmd == HELLO_CS_CMD_SETNAME)
        {
            //IGNORED
        }
        else if (cmd == HELLO_CS_CMD_BROADCAST)
        {
            //IGNORED
        }
    }
};
