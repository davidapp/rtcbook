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
        else if (nCmd == HELLO_CS_CMD_SETNAME) {
            DUInt16 nameLen = rbContent.ReadUInt16(true);
            DBuffer nameBuf = rbContent.ReadFixBuffer(nameLen);
            std::string name((DChar*)nameBuf.GetBuf(), nameBuf.GetSize());
            DUInt32 fromID = pServer->FindIDBySock(sockRecv);
            DBool bOK = pServer->SetIDName(fromID, name);
            if (bOK) {
                ReplyOK(pServer, sockRecv, nCmd);
                pServer->NotifyOtherNameChange(sockRecv, name);
            }
            else {
                ReplyFail(pServer, sockRecv, nCmd);
            }
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
            DGrowBuffer bufAll;
            bufAll.AddUInt32(buf.GetSize(), true);
            bufAll.AddFixBuffer(buf);
            DBuffer bufSend = bufAll.Finish();
            pServer->AsyncSend(sockRecv, bufSend);
        }
        else if (nCmd == HELLO_CS_CMD_SENDTEXT) {
            DUInt32 toID = rbContent.ReadUInt32();
            std::string strText = rbContent.ReadStringA();
            DUInt32 fromID = pServer->FindIDBySock(sockRecv);
            DSocket toSock = pServer->FindSockByID(toID);
            if (toSock != 0)
            {
                pServer->SendOneMsg(toSock, fromID, strText);
                ReplyOK(pServer, sockRecv, nCmd);
            }
            else {
                ReplyFail(pServer, sockRecv, nCmd);
            }
        }
        else if (nCmd == HELLO_CS_CMD_BROADCAST) {
            std::string text = rbContent.ReadStringA();
            pServer->SendGroupMsg(sockRecv, text);
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
        pServer->AsyncSend(sock, buf);
    }

    static DVoid ReplyFail(DTCPServer* pServer, DSocket sock, DUInt32 cmd)
    {
        DGrowBuffer bufReply;
        bufReply.AddUInt32(2, true);
        bufReply.AddUInt8(cmd);
        bufReply.AddUInt8(HELLO_RESULT_FAIL);
        DBuffer buf = bufReply.Finish();
        pServer->AsyncSend(sock, buf);
    }
};
