#include "DTCPServer.h"
#include "Base/DUtil.h"
#include "Net/DNet.h"

DBool DTCPServer::Start(DUInt16 wPort, DUInt16 backlog)
{
    m_state;
    m_wPort = wPort;
    m_backlog = backlog;
    m_state = 0;
    m_clientsMutex.lock();
    m_vecClients.clear();
    m_clientsMutex.unlock();
    m_serverthread.reset(new std::thread(&DTCPServer::ServerLoop, this));
    return true;
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

DTCPSocket DTCPServer::GetClient(DInt32 index)
{
    return m_vecClients[index];
}

DVoid DTCPServer::RemoveClient(DTCPSocket client)
{
    for (auto item = m_vecClients.begin(); item != m_vecClients.end(); item++) {
        if (*item == client) {
            m_vecClients.erase(item);
            break;
        }
    }
}
