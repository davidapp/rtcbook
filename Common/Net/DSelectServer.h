#include "DTypes.h"
#include "DTCPServer.h"
#include <string>
#include <mutex>

#define DEVENT_SOCKET_ERROR 1


class DSelectServer : public DTCPServer
{
public:
    virtual DVoid ServerLoop();
    virtual DVoid Stop();

private:
    DUInt32 m_replyQueue;
};
