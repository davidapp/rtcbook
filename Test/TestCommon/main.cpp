#include "Base/DTestBuffer.h"
#include "Base/DTestUTF8.h"
#include "Base/DTestMsgQueue.h"
#include "Net/DTestTCPClient.h"

int main()
{
    //DTestUTF8::Test();
    //DTestMsgQueue::Use_1q2t();
    //DTestMsgQueue::Use();

    DTestTCPClient::Test();

    return 0;
}
