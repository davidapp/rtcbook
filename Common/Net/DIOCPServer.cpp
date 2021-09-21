#include "DIOCPServer.h"
#include <stdlib.h>
#include <process.h> // for _beginthreadex

typedef struct _PER_HANDLE_DATA
{
    SOCKET Socket;
    SOCKADDR_STORAGE ClientAddr;
} PER_HANDLE_DATA;

#define DATA_BUFSIZE 4096

typedef struct
{
    OVERLAPPED Overlapped;
    DByte Buffer[DATA_BUFSIZE];
    DUInt32 BufferLen;
    DUInt32 OperationType;
} PER_IO_DATA;

HWND g_NotifyWnd;

DBool DIOCPServer::Start(HWND hNotifyWnd, DUInt16 port)
{
    g_NotifyWnd = hNotifyWnd;

    DUInt32 dwThreadID = 0;
    HANDLE hThread = INVALID_HANDLE_VALUE;
    // Windows���� _beginthreadex ��֤��Ӧ��C����ʱ�õ�����
    hThread = (HANDLE)_beginthreadex(NULL, 0, DIOCPServer::ListenThread, (void*)port, 0, &dwThreadID);
    return true;
}

DUInt32 DIOCPServer::ListenThread(DVoid* nPort)
{
    // ���� IO ��ɶ˿ڣ�����һ���̸߳��ͻ���
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    HANDLE CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0,
        SystemInfo.dwNumberOfProcessors - 1);

    // ��Ҫ���߳���Ϊ ����ִ���߳�����2��
    for (int i = 0; i < 2 * (SystemInfo.dwNumberOfProcessors - 1); i++)
    {
        HANDLE ThreadHandle = 0;
        // ����ɶ˿ڵľ������ȥ
        ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, DIOCPServer::ServerWorkerThread, (void*)CompletionPort, 0, NULL);
        if (ThreadHandle != nullptr) {
            CloseHandle(ThreadHandle);
        }
    }

    // ����һ���������� listen socket
    DSocket Listen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    // ����ѡ�Ķ˿ڿ�ʼ����
    SOCKADDR_IN InternetAddr = {};
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons((DUInt16)nPort);
    bind(Listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr));
    int ret = listen(Listen, 5);
    if (ret == 0)
    {
        SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)DEVENT_SERVER_READY, (LPARAM)nPort);
    }

    while (TRUE)
    {
        SOCKADDR_IN saRemote = {};
        int RemoteLen = sizeof(saRemote);
        // �����ȴ����ӵ���
        DSocket Accept = WSAAccept(Listen, (SOCKADDR*)&saRemote, &RemoteLen, NULL, 0);

        // ����һ���������ͽ������ Socket �� IOCP ��������
        if (Accept != NULL)
        {
            PER_HANDLE_DATA* PerHandleData = (PER_HANDLE_DATA*)malloc(sizeof(PER_HANDLE_DATA));
            PerHandleData->Socket = Accept;
            memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);
            CreateIoCompletionPort((HANDLE)Accept, CompletionPort, (DWORD)PerHandleData, 0);

            // ֮�����ͨ�� WSASend WSARecv ȥ�������첽IO����
        }
        // ������һ�μ�����ֱ��֪ͨ������
    }
    return 0;
}

DUInt32 WINAPI DIOCPServer::ServerWorkerThread(LPVOID CompletionPortID)
{
    HANDLE CompletionPort = (HANDLE)CompletionPortID;
    DWORD BytesTransferred;
    LPOVERLAPPED Overlapped;
    PER_HANDLE_DATA* PerHandleData;
    PER_IO_DATA* PerIoData;
    DWORD SendBytes, RecvBytes;
    DWORD Flags;

    while (TRUE)
    {
        // Wait for I/O to complete on any socket
        // associated with the completion port

        BOOL ret = GetQueuedCompletionStatus(CompletionPort,
            &BytesTransferred, (LPDWORD)&PerHandleData,
            (LPOVERLAPPED*)&PerIoData, INFINITE);

        // First check to see if an error has occurred
        // on the socket; if so, close the 
        // socket and clean up the per-handle data
        // and per-I/O operation data associated with
        // the socket
        /*
        if (BytesTransferred == 0 &&
            (PerIoData->OperationType == RECV_POSTED ����
                PerIoData->OperationType == SEND_POSTED))
        {
            // A zero BytesTransferred indicates that the
            // socket has been closed by the peer, so
            // you should close the socket. Note:
            // Per-handle data was used to reference the
            // socket associated with the I/O operation.

            closesocket(PerHandleData->Socket);

            GlobalFree(PerHandleData);
            GlobalFree(PerIoData);
            continue;
        }

        // Service the completed I/O request. You can
        // determine which I/O request has just
        // completed by looking at the OperationType
        // field contained in the per-I/O operation data.
        if (PerIoData->OperationType == RECV_POSTED)
        {
            // Do something with the received data
            // in PerIoData->Buffer
        }

        // Post another WSASend or WSARecv operation.
        // As an example, we will post another WSARecv()
        // I/O operation.

        Flags = 0;

        // Set up the per-I/O operation data for the next
        // overlapped call
        ZeroMemory(&(PerIoData->Overlapped),
            sizeof(OVERLAPPED));

        PerIoData->DataBuf.len = DATA_BUFSIZE;
        PerIoData->DataBuf.buf = PerIoData->Buffer;
        PerIoData->OperationType = RECV_POSTED;

        WSARecv(PerHandleData->Socket,
            &(PerIoData->DataBuf), 1, &RecvBytes,
            &Flags, &(PerIoData->Overlapped), NULL);*/
    }
    return 0;
}


DBool DIOCPServer::Stop()
{
    return true;
}