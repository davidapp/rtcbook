#include "DTypes.h"

#define DEVENT_SERVER_READY 1

class DIOCPServer
{
public:
    static DBool Start(HWND hNotifyWnd, DUInt16 port);
    static DBool Stop();

private:
    static DUInt32 WINAPI ListenThread(DVoid* nPort);
    static DUInt32 WINAPI ServerWorkerThread(DVoid* CompletionPortID);
};
