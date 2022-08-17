#include "DUDPClient.h"
#include "Base/DUtil.h"
#include "Base/DMsgQueue.h"

DVoid* DX86_STDCALL SendToHandler(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    /*
    if (msg == DM_NET_SEND)
    {
        DSendToData* pData = (DSendToData*)para1;
        DUDPClient* sock = (DUDPClient*)para2;
        DBuffer buf;
        buf.Attach(pData->buffer);
        DUDPDataSink* pSink = pData->pSink;
        DChar* pStart = (DChar*)buf.GetBuf();
        DUInt32 size = buf.GetSize();
        if (pSink)
        {
            pSink->OnPreSend(sock, (DByte*)pStart, size);
        }
#if defined(BUILD_FOR_WINDOWS)
        SOCKADDR_IN ReceiverAddr = { 0 };
#else
        struct sockaddr_in ReceiverAddr = { 0 };
#endif
        ReceiverAddr.sin_family = AF_INET;
        ReceiverAddr.sin_port = htons(pData->wPort);
        inet_pton(AF_INET, pData->strIP, &ReceiverAddr.sin_addr.s_addr);

        DUInt32 sent = 0;
        while (sent < size)
        {
#if defined(BUILD_FOR_WINDOWS)
            DInt32 ret = sendto(pData->sock, pStart, size - sent, 0, (SOCKADDR*)&ReceiverAddr, sizeof(ReceiverAddr));
#else
            DInt32 ret = (DInt32)sendto(pData->sock, pStart, size - sent, 0, (sockaddr*)&ReceiverAddr, sizeof(ReceiverAddr));
#endif
            if (ret == DSockError)
            {
                DUInt32 errCode = DError::GetLastNetError();
                DStringA strReasonA = DError::GetLastNetErrorStr();
                if (pSink)
                {
                    pSink->OnSendError(sock, errCode, strReasonA);
                }
                return NULL;
            }
            sent += ret;
            pStart += ret;
        }
        if (pSink)
        {
            pSink->OnSendOK(sock);
            sock->Recv();
        }
        DStringA strA;
        strA.Attach(pData->strIP);

        delete pData;
    }
    */
    return NULL;
}
