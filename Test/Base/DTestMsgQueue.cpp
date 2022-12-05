#include "DTestMsgQueue.h"
#include "Base/DMsgQueue.h"
#include <iostream>
#include "Base/DAtomic.h"

#define WM_MYPRINT 1

DAtomInt32 g_counter;

DVoid* DX86_STDCALL DPrint1(DUInt32 msg, DVoid* para1, DVoid* para2)
{
    if (msg == WM_MYPRINT) {
        DInt32 tid = (DInt32)para2;
        if (tid == 1) {
            //printf("%d's %p\n", tid, para1);
            g_counter++;
        }
        else if (tid == 2)
        {
            //printf("%d's %p\n", tid, para1);
            g_counter+=1000;
        }

        if (g_counter == 1001000) {
            printf("Testing Finished OK!\n");
            printf("Queue's size=%d\n", DMsgQueue::GetQueueSize(1));
        }
    }
    return nullptr;
}

void test_thread(DUInt32 q1, DInt32 tid) 
{
    printf("%d is filling queue\n", tid);
    for (DUInt32 i = 0; i < 1000;) {
        DUInt32 nRes = DMsgQueue::PostQueueMsg(q1, WM_MYPRINT, (DVoid*)i, (DVoid*)tid);
        if (nRes == 2) {
            printf("Queue is full, i=%u, tid=%d\n", i, tid);
            printf("Queue's size=%d\n", DMsgQueue::GetQueueSize(1));
            Sleep(10);
        }
        else {
            i++;
        }
    }
}

// 1个队列长度100 2个线程Post1000 压力测试
DVoid DTestMsgQueue::Use_1q2t()
{
    DUInt32 q1 = DMsgQueue::Create("queue1", 100);
    DMsgQueue::AddHandler(q1, DPrint1);

    std::thread t = std::thread(test_thread, q1, 1);
    t.detach();
    std::thread t2 = std::thread(test_thread, q1, 2);
    t2.detach();

    int a;
    std::cin >> a;
}

DVoid DTestMsgQueue::Use()
{
    std::cout << DMsgQueue::GetCoreCount() << std::endl;
}


DVoid DTestMsgQueue::Test()
{

}
