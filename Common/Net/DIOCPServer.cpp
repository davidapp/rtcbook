#include "DIOCPServer.h"
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <vector>

typedef struct _PER_HANDLE_DATA
{
    SOCKET Socket;
    SOCKADDR_STORAGE ClientAddr;
} PER_HANDLE_DATA;

#define DATA_BUFSIZE 4096
#define RECV_POSTED 1
#define SEND_POSTED 2
typedef struct
{
    OVERLAPPED Overlapped;
    DByte Buffer[DATA_BUFSIZE];
    DUInt32 BufferLen;
    DUInt32 OperationType;
} PER_IO_DATA;

HWND g_NotifyWnd;


std::vector<PER_HANDLE_DATA*> g_clients;
std::mutex g_clients_guard;


DBool DIOCPServer::Start(HWND hNotifyWnd, DUInt16 port)
{
    g_NotifyWnd = hNotifyWnd;
    std::thread listen(DIOCPServer::ListenThread, port);
    listen.detach();
    return true;
}

DBool DIOCPServer::Stop()
{
    return true;
}

std::string DIOCPServer::Info()
{
    return "";
}

DUInt32 DIOCPServer::ListenThread(DUInt16 nPort)
{
    // ���� IO ��ɶ˿ڣ�����һ���̸߳��ͻ���
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    HANDLE CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, SystemInfo.dwNumberOfProcessors - 1);
    if (CompletionPort == NULL) {
        SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_IOCP_ERROR, 0);
        return DEVENT_IOCP_ERROR;
    }

    // ��Ҫ���߳���Ϊ ����ִ���߳�����2��
    for (DUInt32 i = 0; i < 2 * (SystemInfo.dwNumberOfProcessors - 1); i++)
    {
        std::thread worker(DIOCPServer::ServerWorkerThread, (DVoid*)CompletionPort);
        worker.detach();
    }

    // ����һ���������� listen socket
    DSocket Listen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (Listen == INVALID_SOCKET) {
        SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_SOCKET_ERROR, 0);
        return DEVENT_SOCKET_ERROR;
    }

    // ����ѡ�Ķ˿ڿ�ʼ����
    SOCKADDR_IN InternetAddr = {};
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(nPort);
    int ret = bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr));
    if (ret != 0) {
        SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_BIND_ERROR, 0);
        return DEVENT_BIND_ERROR;
    }

    ret = listen(Listen, 5);
    if (ret == 0) {
        SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_SERVER_READY, (LPARAM)nPort);
    }
    else {
        SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_LISTEN_ERROR, (LPARAM)nPort);
        return DEVENT_LISTEN_ERROR;
    }

    while (TRUE)
    {
        // �����ȴ������ӵ���
        SOCKADDR_IN saRemote = {};
        int RemoteLen = sizeof(saRemote);
        DSocket Accept = WSAAccept(Listen, (SOCKADDR*)&saRemote, &RemoteLen, NULL, 0);

        // ����һ���������ͽ������ Socket �� IOCP ��������
        if (Accept != INVALID_SOCKET)
        {
            SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_SERVER_NEWCONN, (LPARAM)nPort);

            PER_HANDLE_DATA* PerHandleData = (PER_HANDLE_DATA*)malloc(sizeof(PER_HANDLE_DATA));
            PerHandleData->Socket = Accept;
            memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);
            if (CreateIoCompletionPort((HANDLE)Accept, CompletionPort, (DWORD)PerHandleData, 0) == NULL)
            {
                SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_ASSOCIATE_ERROR, (LPARAM)PerHandleData);
                closesocket(Accept);
                free(PerHandleData);
            }
            else {
                g_clients_guard.lock();
                g_clients.push_back(PerHandleData);
                g_clients_guard.unlock();
            }
        }
        else
        {
            SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_ACCEPT_ERROR, 0);
        }
        // ������һ�μ�����ֱ��֪ͨ������
    }
    return 0;
}

DUInt32 DIOCPServer::ServerWorkerThread(DVoid* CompletionPortID)
{
    HANDLE CompletionPort = (HANDLE)CompletionPortID;

    while (TRUE)
    {
        // �ȴ� Socket I/O ��ɵ�֪ͨ
        DWORD BytesTransferred;
        PER_HANDLE_DATA* PerHandleData;
        PER_IO_DATA* PerIoData;
        BOOL ret = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (LPDWORD)&PerHandleData,
            (LPOVERLAPPED*)&PerIoData, INFINITE);

        // ��������˶�д���󣬾͹رմ� Socket ����
        if (BytesTransferred == 0 && (PerIoData->OperationType == RECV_POSTED || PerIoData->OperationType == SEND_POSTED))
        {
            closesocket(PerHandleData->Socket);
            free(PerHandleData);
            free(PerIoData);
            continue;
        }

        // �յ�һ������
        if (PerIoData->OperationType == RECV_POSTED)
        {

        }
    }
    return 0;
}
