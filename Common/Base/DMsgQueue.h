#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "Base/DUtil.h"
#include <list>
#include <mutex>
#include <thread>

typedef DVoid* (DX86_STDCALL* DMsgFunc)(DUInt32, DVoid*, DVoid*);

typedef struct tagDQMsg
{
    DUInt32 msg;
    DVoid* para1;
    DVoid* para2;
}DQMsg;

//Msgs
#define DM_QUITMSG    0
#define DM_BROADCAST  1
#define DM_TESTPRINT  2
#define DM_TESTPRINT2 3

#define DM_NET_CONN   1000
#define DM_NET_SEND   1001
#define DM_TASK_MSG   1002

#define D_WM_HANDLER  WM_USER+1


class DMsgQueue
{
public:
    static DHandle Create(DCStr queueName, DUInt32 maxSize = 500);
    static DHandle GetQueue(DCStr queueName);
    static DUInt32 PostQueueMsg(DHandle qid, DUInt32 msg, DVoid* para1, DVoid* para2);
    static DUInt32 SendQueueMsg(DHandle qid, DUInt32 msg, DVoid* para1, DVoid* para2);
    static DVoid PostQuitMsg(DHandle qid);

    static DBool   IsInQueue(DHandle qid);
    static DHandle GetCurQueueID();
    static DCStr   GetCurQueueName();
    static DUInt32 GetCoreCount();

    //Msg Handler
    static DVoid AddHandler(DHandle qid, DMsgFunc handler);
    static DVoid RemoveHandler(DHandle qid, DMsgFunc handler);
    static DVoid RemoveAllHandler(DHandle qid);

public:
    DMsgQueue(DCStr queueName, DUInt32 maxSize);
    ~DMsgQueue() = default;
    std::list<DQMsg> m_queue;
    std::list<DMsgFunc> m_msgfunc;
    std::mutex m_queueMutex;
    std::thread m_t;
    std::string m_name;
    DEvent m_wait;
    DUInt32 m_maxSize;

public:
    D_DISALLOW_COPY_AND_ASSIGN(DMsgQueue)
};
