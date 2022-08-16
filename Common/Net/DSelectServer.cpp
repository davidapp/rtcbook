#include "DSelectServer.h"
#include "DTCPClient.h"
#include "DNet.h"
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>

DVoid DSelectServer::ServerLoop()
{
    DTCPSocket serverSocket;
    if (!serverSocket.Create())
    {
        if (m_pListenSink) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pListenSink->OnListenError(this, errCode, strReason);
        }
        return;
    }

    if (!serverSocket.Bind(m_wPort)) {
        if (m_pListenSink) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pListenSink->OnListenError(this, errCode, strReason);
        }
    }
    else {
        if (m_pListenSink) {
            m_pListenSink->OnListening(this, m_wPort);
        }
    }

    if (serverSocket.Listen(5)) {
        if (m_pListenSink) {
            m_pListenSink->OnListenOK(this, m_wPort);
        }
    }
    else {
        if (m_pListenSink) {
            DUInt32 errCode = DNet::GetLastNetError();
            std::string strReason = DNet::GetLastNetErrorStr(errCode);
            m_pListenSink->OnListenError(this, errCode, strReason);
        }
    }

    fd_set fdread;
    // 1.Data is available for reading.
    // 2.Connection has been closed, reset, or terminated.
    // 3.If listen has been called and a connection is pending, the accept function will succeed.
    fd_set fdwrite;
    // 1.Data can be sent.
    // 2.If a non - blocking connect call is being processed, the connection has succeeded.
    fd_set fdexp;
    // 1.If a non-blocking connect call is being processed, the connection attempt failed.
    // 2.OOB data is available for reading.
    timeval select_timeout = { 10, 0 };
    int ret = 0;
    while (TRUE)
    {
        FD_ZERO(&fdread);
        FD_SET(serverSocket.m_sock, &fdread);
        for (auto item = m_vecClients.begin(); item != m_vecClients.end(); item++) {
            FD_SET(item->m_sock, &fdread);
        }

        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexp);

        if ((ret = select(0, &fdread, NULL, NULL, &select_timeout)) == SOCKET_ERROR)
        {
            // Error condition
        }

        if (ret > 0)
        {
            if (FD_ISSET(serverSocket.m_sock, &fdread))
            {
                DTCPSocket client = serverSocket.Accept();
                m_vecClients.push_back(client);
            }
            else {
                for (auto item = m_vecClients.begin(); item != m_vecClients.end(); item++) {
                    if (FD_ISSET(item->m_sock, &fdread))
                    {
                        DTCPSocket client;
                        client.Attach(item->m_sock);
                        DUInt32 res;
                        DBuffer buf = client.SyncRecv(4, &res);

                    }
                }
            }
        }
    }
}