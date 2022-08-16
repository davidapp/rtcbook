#include "DSelectServer.h"
#include "DTCPClient.h"
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>

std::vector<DTCPSocket> g_clients;

DBool DSelectServer::Start(DUInt16 port)
{
    std::thread listen(DSelectServer::ServerListenThread, port);
    listen.detach();
    return true;
}

DBool DSelectServer::Stop()
{
    return true;
}

std::string DSelectServer::Info()
{
    return "";
}

DUInt32 DSelectServer::GetMaxFDSetSize()
{
    return FD_SETSIZE;
}

DUInt32 DSelectServer::ServerListenThread(DUInt16 nPort)
{
    DTCPSocket serverSocket;
    if (!serverSocket.Create())
    {
        return DEVENT_SOCKET_ERROR;
    }

    if (!serverSocket.Bind(nPort)) {
        //SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_BIND_ERROR, 0);
        return DEVENT_BIND_ERROR;
    }

    if (serverSocket.Listen(5)) {
        //SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_SERVER_READY, (LPARAM)nPort);
    }
    else {
        //SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_LISTEN_ERROR, (LPARAM)nPort);
        return DEVENT_LISTEN_ERROR;
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
    timeval select_timeout = {10, 0};
    int ret = 0;
    while (TRUE)
    {
        FD_ZERO(&fdread);
        FD_SET(serverSocket.m_sock, &fdread);
        for (auto item = g_clients.begin(); item != g_clients.end(); item++) {
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
                g_clients.push_back(client);
            }
            else {
                for (auto item = g_clients.begin(); item != g_clients.end(); item++) {
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
    return 0;
}
