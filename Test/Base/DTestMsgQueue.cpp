#include "DTestMsgQueue.h"
#include "Base/DMsgQueue.h"
#include <iostream>

DVoid* DX86_STDCALL DPrint1(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    if (DMsgQueue::IsInQueue(1)) {
        printf("[queue1][print1] %u %d %p\n", msg, (DUInt32)para1, para2);
    }
    else if (DMsgQueue::IsInQueue(2)) {
        printf("[queue2][print1] %u %d %p\n", msg, (DUInt32)para1, para2);
    }
    return nullptr;
}

DVoid* DX86_STDCALL DPrint2(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    if (DMsgQueue::IsInQueue(1)) {
        printf("[queue1][print2] %u %d %p\n", msg, (DUInt32)para1, para2);
    }
    else if (DMsgQueue::IsInQueue(2)) {
        printf("[queue2][print2] %u %d %p\n", msg, (DUInt32)para1, para2);
    }
    return nullptr;
}

void test_thread(DUInt32 q1, DUInt32 q2) 
{
    printf("filling q1\n");
    for (DUInt32 i = 0; i < 1000;) {
        DBool bOK = DMsgQueue::PostQueueMsg(q1, 1, (DVoid*)i, 0);
        if (!bOK) {
            printf("q1 is full, i=%u\n", i);
            Sleep(10);
        }
        else {
            i++;
        }
    }
}

// 1个队列 2个线程 压力测试
DVoid DTestMsgQueue::Use_1q2t()
{
    DUInt32 q1 = DMsgQueue::Create("queue1", 100);
    DMsgQueue::AddHandler(q1, DPrint1);
    DMsgQueue::AddHandler(q1, DPrint2);

    DUInt32 q2 = DMsgQueue::Create("queue2", 50);
    DMsgQueue::AddHandler(q2, DPrint1);
    DMsgQueue::AddHandler(q2, DPrint2);

    std::thread t = std::thread(test_thread, q1, q2);
    t.detach();
    std::thread t2 = std::thread(test_thread, q1, q2);
    t2.detach();

    int a;
    std::cin >> a;
}

DVoid DTestMsgQueue::Use()
{
    std::cout << DMsgQueue::GetCoreCount() << std::endl;
}