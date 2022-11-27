#pragma once

#include "Protocol/DHelloProtocol.h"
#include "Net/DTCPServer.h"

class DHelloServer
{
public:
    static DVoid Process(DTCPServer* pServer, DSocket sockRecv, DBuffer bufRecv)
    {
        DReadBuffer rbContent(bufRecv);
        DUInt32 nCmd = rbContent.ReadUInt8();
        if (nCmd == HELLO_CS_CMD_HEARTBEAT) {
            ReplyOK(pServer, sockRecv, nCmd);
        }
        else if (nCmd == HELLO_CS_CMD_GETINFO) {
            DGrowBuffer bufData;
            bufData.AddUInt8(HELLO_CS_CMD_GETINFO);
            bufData.AddUInt8(HELLO_RESULT_SUCCESS);
            DUInt32 nCount = pServer->GetClientCount();
            bufData.AddUInt16((DUInt16)nCount, true);
            for (DUInt32 i = 0; i < nCount; i++)
            {
                DClientData cdata = pServer->GetClient(i);
                bufData.AddUInt32(cdata.m_id, true);
                DUInt16 name_len = (DUInt16)cdata.m_name.size();
                bufData.AddUInt16(name_len, true);
                DBuffer name_buf((DByte*)cdata.m_name.c_str(), name_len);
                bufData.AddFixBuffer(name_buf);
            }
            DBuffer buf = bufData.Finish();
            pServer->ReplyOne(sockRecv, buf);
        }
        else if (nCmd == HELLO_CS_CMD_SENDTEXT) {
            DUInt32 toID = rbContent.ReadUInt32();
            std::string strText = rbContent.ReadStringA();
            DUInt32 fromID = pServer->FindIDBySock(sockRecv);
            DSocket toSock = pServer->FindSockByID(toID);
            if (toSock != 0)
            {
                SendSCPMsg(toSock, fromID, strText);
                ReplyOK(pServer, sockRecv, nCmd);
            }
            else {
                ReplyFail(pServer, sockRecv, nCmd);
            }
        }
        else if (nCmd == HELLO_CS_CMD_SETNAME) {
            DUInt16 nameLen = rbContent.ReadUInt16(true);
            DBuffer nameBuf = rbContent.ReadFixBuffer(nameLen);
            std::string name((DChar*)nameBuf.GetBuf(), nameBuf.GetSize());
            DUInt32 fromID = pServer->FindIDBySock(sockRecv);
            DBool bOK = pServer->SetIDName(fromID, name);
            if (bOK) {
                ReplyOK(pServer, sockRecv, nCmd);
            }
            else {
                ReplyFail(pServer, sockRecv, nCmd);
            }
        }
        else if (nCmd == HELLO_CS_CMD_BROADCAST) {
            std::string text = rbContent.ReadStringA();
            DBuffer bufText((DByte*)text.c_str(), text.size());
            pServer->ReplyAll(sockRecv, bufText);
            ReplyOK(pServer, sockRecv, nCmd);
        }
    }

    static DVoid ReplyOK(DTCPServer* pServer, DSocket sock, DUInt32 cmd)
    {
        DGrowBuffer bufReply;
        bufReply.AddUInt32(2, true);
        bufReply.AddUInt8(cmd);
        bufReply.AddUInt8(HELLO_RESULT_SUCCESS);
        DBuffer buf = bufReply.Finish();
        pServer->ReplyOne(sock, buf);
    }

    static DVoid ReplyFail(DTCPServer* pServer, DSocket sock, DUInt32 cmd)
    {
        DGrowBuffer bufReply;
        bufReply.AddUInt32(2, true);
        bufReply.AddUInt8(cmd);
        bufReply.AddUInt8(HELLO_RESULT_FAIL);
        DBuffer buf = bufReply.Finish();
        pServer->ReplyOne(sock, buf);
    }


    static DBool SendSCEnter(DSocket toSock, DUInt32 fromID, std::string fromName)
    {
        DTCPSocket sock(toSock);
        DGrowBuffer gb;
        gb.AddUInt32(1 + 4 + 2 + fromName.size(), true);
        gb.AddUInt8(HELLO_SC_CMD_ENTER);
        gb.AddUInt32(fromID, true);
        gb.AddUInt16((DUInt16)fromName.size(), true);
        DBuffer bufName((DByte*)fromName.c_str(), (DUInt16)fromName.size());
        gb.AddFixBuffer(bufName);
        DBuffer bufSend = gb.Finish();
        sock.SyncSend(bufSend);
        return true;
    }

    static DBool SendSCLeave(DSocket toSock, DUInt32 fromID, std::string fromName)
    {
        DTCPSocket sock(toSock);
        DGrowBuffer gb;
        gb.AddUInt32(1 + 4 + 2 + fromName.size(), true);
        gb.AddUInt8(HELLO_SC_CMD_LEAVE);
        gb.AddUInt32(fromID, true);
        gb.AddUInt16((DUInt16)fromName.size(), true);
        DBuffer bufName((DByte*)fromName.c_str(), (DUInt16)fromName.size());
        gb.AddFixBuffer(bufName);
        DBuffer bufSend = gb.Finish();
        sock.SyncSend(bufSend);
        return true;
    }

    static DBool SendSCPMsg(DSocket toSock, DUInt32 fromID, std::string text)
    {
        DTCPSocket sock(toSock);
        DGrowBuffer gb;
        gb.AddUInt32(1 + 4 + 4 + text.size(), true);
        gb.AddUInt8(HELLO_SC_CMD_PMSG);
        gb.AddUInt32(fromID, true);
        gb.AddStringA(text);
        DBuffer bufSend = gb.Finish();
        sock.SyncSend(bufSend);
        return true;
    }

    static DBool SendSCGMsg(DSocket toSock, DUInt32 fromID, std::string text)
    {
        DTCPSocket sock(toSock);
        DGrowBuffer gb;
        gb.AddUInt32(1 + 4 + 4 + text.size(), true);
        gb.AddUInt8(HELLO_SC_CMD_GMSG);
        gb.AddUInt32(fromID, true);
        gb.AddStringA(text);
        DBuffer bufSend = gb.Finish();
        sock.SyncSend(bufSend);
        return true;
    }
};
