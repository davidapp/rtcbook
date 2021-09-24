#include "DTypes.h"
#include <string>

#define DEVENT_IOCP_ERROR 1
#define DEVENT_SOCKET_ERROR 2
#define DEVENT_BIND_ERROR 3
#define DEVENT_LISTEN_ERROR 4
#define DEVENT_ACCEPT_ERROR 5
#define DEVENT_ASSOCIATE_ERROR 6

#define DEVENT_SERVER_READY 100
#define DEVENT_SERVER_NEWCONN 101

class DIOCPServer
{
public:
    static DBool Start(HWND hNotifyWnd, DUInt16 port);
    static DBool Stop();
    static std::string Info();

private:
    static DUInt32 ListenThread(DUInt16 nPort);
    static DUInt32 ServerWorkerThread(DVoid* CompletionPortID);
};
