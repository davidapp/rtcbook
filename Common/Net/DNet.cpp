#include "DNet.h"

#if defined(BUILD_FOR_WINDOWS)
#include <winsock2.H>
#pragma comment(lib, "ws2_32.lib")
WSADATA g_wsaData;
#else
#endif

DBool DNet::Init()
{
#if defined(BUILD_FOR_WINDOWS)
	if (WSAStartup(MAKEWORD(2, 2),  &g_wsaData) != 0)
	{
		return false;
	}
#endif
	return true;
}

DBool DNet::UnInit()
{
#if defined(BUILD_FOR_WINDOWS)
	if (WSACleanup() == SOCKET_ERROR)
	{
		return false;
	}
#endif
	return true;
}

DUInt16 DNet::Swap16(DUInt16 h)
{
    DUInt16 ret;
    DByte* p1 = (DByte*)&h;
    DByte* p2 = (DByte*)&ret;
    p2[0] = p1[1];
    p2[1] = p1[0];
    return ret;
}

DUInt32 DNet::Swap32(DUInt32 h)
{
    DUInt32 ret;
    DByte* p1 = (DByte*)&h;
    DByte* p2 = (DByte*)&ret;
    p2[0] = p1[3];
    p2[1] = p1[2];
    p2[2] = p1[1];
    p2[3] = p1[0];
    return ret;
}