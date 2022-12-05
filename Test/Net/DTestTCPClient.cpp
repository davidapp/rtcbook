#include "DTestTCPClient.h"
#include "Base/DTest.h"
#include "Net/DTCPClient.h"
#include <iostream>

DVoid DTestTCPClient::Test()
{
    std::string cmd;
    while (1) {
        std::cin >> cmd;
        if (cmd == "q") break;
    }
    std::cout << "Test Finished." << std::endl;
}
