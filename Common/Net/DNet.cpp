#include "DNet.h"

#if defined(BUILD_FOR_WINDOWS)
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
