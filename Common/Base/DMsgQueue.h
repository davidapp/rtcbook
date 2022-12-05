#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "Base/DUtil.h"
#include <list>
#include <mutex>
#include <thread>
#include <memory>

typedef DVoid* (DX86_STDCALL* DMsgFunc)(DUInt32, DVoid*, DVoid*);

typedef struct tagDQMsg
{
    DUInt32 msg;
    DVoid* para1;
    DVoid* para2;
}DQMsg;


class DMsgQueue
{
public:
    static DHandle Create(DCStr queueName, DUInt32 maxSize = 500);
    static DHandle GetQueue(DCStr queueName);
    static DVoid   RemoveQueue(DHandle qid);

    static DUInt32 PostQueueMsg(DHandle qid, DUInt32 msg, DVoid* para1, DVoid* para2);
    static DVoid   PostQuitMsg(DHandle qid);

    static DVoid   Quit(DHandle qid);
    static DVoid*  GetThreadHandle(DHandle qid);

    static DBool   IsInQueue(DHandle qid);
    static DHandle GetCurQueueID();
    static DCStr   GetCurQueueName();
    static DUInt32 GetCoreCount();

    // Msg Handler
    static DVoid AddHandler(DHandle qid, DMsgFunc handler);
    static DVoid RemoveHandler(DHandle qid, DMsgFunc handler);
    static DVoid RemoveAllHandler(DHandle qid);

    // Msg Cleaner
    static DVoid SetCleaner(DHandle qid, DMsgFunc handler);
    static DVoid ClearAllMsg(DHandle qid);

public:
    DMsgQueue(DCStr queueName, DUInt32 maxSize);
    ~DMsgQueue() = default;

    std::list<DQMsg> m_queue;
    std::mutex m_queueMutex;

    std::list<DMsgFunc> m_msgfunc;
    std::mutex m_msgfuncMutex;
    DMsgFunc m_msgcleaner;

    std::shared_ptr<std::thread> m_t;
    std::string m_name;
    DEvent m_wait;
    DUInt32 m_maxSize;

public:
    D_DISALLOW_COPY_AND_ASSIGN(DMsgQueue)
};
