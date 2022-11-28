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
        gb.AddUInt32(1 + 4 + 4 + text.size(), true);
        gb.AddUInt8(HELLO_CS_CMD_SENDTEXT);
        gb.AddUInt32(id, true);
        gb.AddStringA(text);
        DBuffer buf = gb.Finish();
        pClient->Send(buf);
    }

    static DVoid SendSetName(DTCPClient* pClient, std::string name)
    {
        DGrowBuffer gb;
        gb.AddUInt32(1 + 2 + name.size(), true);
        gb.AddUInt8(HELLO_CS_CMD_SETNAME);
        gb.AddUInt16((DUInt16)name.size(), true);
        DBuffer bufname((DByte*)name.c_str(), name.size());
        gb.AddFixBuffer(bufname);
        DBuffer buf = gb.Finish();
        pClient->Send(buf);
    }

    static DVoid SendAll(DTCPClient* pClient, std::string text)
    {
        DGrowBuffer gb;
        gb.AddUInt32(1 + 4 + text.size(), true);
        gb.AddUInt8(HELLO_CS_CMD_BROADCAST);
        gb.AddStringA(text);
        DBuffer buf = gb.Finish();
        pClient->Send(buf);
    }

    static std::string HandleRecvBuffer(HWND hWnd, DSocket sock, DBuffer recvBuf, std::map<DUInt32, std::string>& data)
    {
        std::string strRet;
        DReadBuffer rb(recvBuf);
        DUInt8 cmd = rb.ReadUInt8();
        if (cmd == HELLO_CS_CMD_HEARTBEAT)
        {
            //IGNORED
        }
        else if (cmd == HELLO_CS_CMD_SETNAME)
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
                    data.insert(std::make_pair(nID, strName));
                }
            }
        }
        else if (cmd == HELLO_CS_CMD_SENDTEXT)
        {
            //IGNORED
        }
        else if (cmd == HELLO_CS_CMD_BROADCAST)
        {
            //IGNORED
        }
        // Server 主动下推的消息
        else if (cmd == HELLO_SC_CMD_CNAME)
        {
            DUInt32 userID = rb.ReadUInt32(true);
            DBool bFind = false;
            std::string oldname;
            for (auto i = data.begin(); i != data.end(); i++)
            {
                if ((*i).first == userID)
                {
                    bFind = true;
                    oldname = i->second;
                }
            }
            DUInt16 nameLen = rb.ReadUInt16(true);
            DBuffer nameBuf = rb.ReadFixBuffer(nameLen);
            std::string strName((DChar*)nameBuf.GetBuf(), nameBuf.GetSize());
            if (bFind) {
                strRet = oldname + " has changed name to " + strName;
            }
            else {
                strRet = strName + " has entered the room.";
            }
            ::PostMessage(hWnd, WM_USER + 1004, 0, 0);
        }
        else if (cmd == HELLO_SC_CMD_LEAVE)
        {
            DUInt32 userID = rb.ReadUInt32(true);
            DBool bFind = false;
            std::string name;
            for (auto i = data.begin(); i != data.end(); i++)
            {
                if ((*i).first == userID)
                {
                    bFind = true;
                    name = i->first;
                }
            }
            if (bFind) {
                strRet = name + " has left the room.";
            }
            ::PostMessage(hWnd, WM_USER + 1004, 0, 0);
        }
        else if (cmd == HELLO_SC_CMD_PMSG)
        {
            DUInt32 userID = rb.ReadUInt32(true);
            std::string text = rb.ReadStringA();
            DBool bFind = false;
            std::string name;
            for (auto i = data.begin(); i != data.end(); i++)
            {
                if ((*i).first == userID)
                {
                    bFind = true;
                    name = i->second;
                }
            }
            if (bFind) {
                strRet = name + " sent to you: " + text;
            }
        }
        else if (cmd == HELLO_SC_CMD_GMSG)
        {
            DUInt32 userID = rb.ReadUInt32(true);
            std::string text = rb.ReadStringA();
            DBool bFind = false;
            std::string name;
            for (auto i = data.begin(); i != data.end(); i++)
            {
                if ((*i).first == userID)
                {
                    bFind = true;
                    name = i->second;
                }
            }
            if (bFind) {
                strRet = name + " sent to all: " + text;
            }
        }
        return strRet;
    }
};
