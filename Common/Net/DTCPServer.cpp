#include "DTCPServer.h"
#include "Base/DUtil.h"
#include "Net/DNet.h"

DBool DTCPServer::Start(DUInt16 wPort, DUInt16 backlog)
{
    m_wPort = wPort;
    m_backlog = backlog;
    std::thread listen(DTCPServer::ServerThread, this);
    listen.detach();
    return true;
}

DUInt32 DTCPServer::ServerThread(DVoid* pThis)
{
    DTCPServer* pThisServer = (DTCPServer*)pThis;
    pThisServer->ServerLoop();
}

DVoid DTCPServer::ServerLoop()
{

}

DVoid DTCPServer::Stop()
{

}

DVoid DTCPServer::SetListenSink(DTCPServerSink* pSink)
{
    m_pListenSink = pSink;
}

DVoid DTCPServer::SetDataSink(DTCPDataSink* pSink)
{
    m_pDataSink = pSink;
}

DUInt32 DTCPServer::GetClientCount()
{
    return m_vecClients.size();
}

DTCPClient* DTCPServer::GetClient(DInt32 index)
{
    return m_vecClients[index];
}

DVoid DTCPServer::RemoveClient(DTCPClient* client)
{
    for (auto item = m_vecClients.begin(); item != m_vecClients.end(); item++) {
        if (*item == client) {
            m_vecClients.erase(item);
            break;
        }
    }
}
