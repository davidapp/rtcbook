#include "DTestTCPClient.h"
#include "Base/DTest.h"
#include "Base/DUTF8.h"
#include "Net/DNet.h"
#include "Net/DTCPClient.h"
#include "Protocol/DHelloClient.h"
#include <iostream>
#include <map>

class DHelloSink : public DTCPClientSink
{
public:
    DHelloSink() {};

    virtual DVoid OnConnecting(DSocket sock, std::string strIP, DUInt16 wPort)
    {

    }

    virtual DVoid OnConnectOK(DSocket sock)
    {
        printf("Connected OK\n");
    }

    virtual DVoid OnConnectError(DSocket sock, DUInt32 code, std::string strReason)
    {
        printf("Connected Error:%d %s\n", code, strReason.c_str());
    }

    // DTCPDataSink
    virtual DVoid OnPreSend(DSocket sock, DBuffer buffer)
    {

    }

    virtual DVoid OnSendOK(DSocket sock)
    {
        printf("OnSendOK()\n");
    }

    virtual DVoid OnSendError(DSocket sock, DUInt32 code, std::string strReason)
    {

    }

    virtual DVoid OnRecvBuf(DSocket sock, DBuffer buf)
    {
        printf("OnRecvBuf()\n");
        std::map<DUInt32, std::string> users;
        DHelloClient::HandleRecvBuffer(NULL, sock, buf, users);
        for (auto i = users.begin(); i != users.end(); i++)
        {
            printf("%s\n", i->second.c_str());
        }

    }

    virtual DVoid OnClose(DSocket sock)
    {
        printf("OnClose()\n");
    }

    virtual DVoid OnBroken(DSocket sock, DUInt32 code, std::string strReason)
    {
        printf("OnBroken()\n");
    }
};

DVoid DTestTCPClient::Test()
{
    DNet::Init();

    DTCPClient m_client;
    DHelloSink* pSink = new DHelloSink();
    m_client.Init();
    m_client.SetSink(pSink);

    std::string cmd;
    std::cout << "Please Enter the cmd (c,d,s,n,l,q)>";
    while (1) {
        std::cin >> cmd;
        if (cmd == "q") break;
        else if (cmd == "c") {
            //Connect
            //m_client.Connect("127.0.0.1", 1229);
            m_client.Connect("192.168.1.18", 1229);
        }
        else if (cmd == "d") {
            //Disconnect
            m_client.DisConnect();
        }
        else if (cmd == "n") {
            DHelloClient::SendSetName(&m_client, "Cmd_001");
        }
        else if (cmd == "s") {
            //Send All
            std::string text;
            std::cin >> text;
            DHelloClient::SendAll(&m_client, text);
        }
        else if (cmd == "l") {
            //List
            DHelloClient::SendGetInfo(&m_client);
        }
        else {
            std::cout << std::endl << "Please Enter the cmd (c,d,s,l,q)>";
        }
    }
    std::cout << "Test Finished." << std::endl;

    delete pSink;
    m_client.UnInit();
    DNet::UnInit();
}
