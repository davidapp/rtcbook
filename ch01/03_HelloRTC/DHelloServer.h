#pragma once

#include "Protocol/DHelloProtocol.h"
#include "Net/DTCPServer.h"

class DHelloServer
{
public:
    static DVoid Process(DTCPServer* pServer, DSocket sockRecv, DBuffer bufRecv)
    {
        DBuffer bufRet;
        DReadBuffer rbContent(bufRecv);
        DUInt32 nCmd = rbContent.ReadUInt8();
        if (nCmd == HELLO_CS_CMD_HEARTBEAT) {
            DGrowBuffer bufReply;
            bufReply.AddUInt32(2, true);
            bufReply.AddUInt8(HELLO_CS_CMD_HEARTBEAT);
            bufReply.AddUInt8(HELLO_RESULT_SUCCESS);
            DBuffer buf = bufReply.Finish();
            pServer->ReplyOne(sockRecv, buf);
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
            DGrowBuffer bufData;
            bufData.AddUInt8(HELLO_CS_CMD_GETINFO);
            DSocket toSock = pServer->FindSockByID(toID);
            if (toSock != 0)
            {
                bufData.AddUInt8(HELLO_RESULT_SUCCESS);
                SendSCPMsg(toSock, fromID, strText);
            }
            else {
                bufData.AddUInt8(HELLO_RESULT_FAIL);
            }
            DBuffer buf = bufData.Finish();
            pServer->ReplyOne(sockRecv, buf);
        }
        else if (nCmd == HELLO_CS_CMD_SETNAME) {

        }
        else if (nCmd == HELLO_CS_CMD_BROADCAST) {

        }
        else if (nCmd == HELLO_CS_CMD_PUSH) {

        }
    }

    static DBool SendSCEnter(DSocket toSock, DUInt32 fromID, std::string fromName)
    {
        return true;
    }

    static DBool SendSCLeave(DSocket toSock, DUInt32 fromID, std::string fromName)
    {
        return true;
    }

    static DBool SendSCPMsg(DSocket toSock, DUInt32 fromID, std::string text)
    {
        return true;
    }

    static DBool SendSCGMsg(DSocket toSock, DUInt32 fromID, std::string text)
    {
        return true;
    }
};
