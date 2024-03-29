﻿#include "DMsgQueue.h"
#include <map>

std::mutex g_id2qMutex;
std::map<DHandle, DMsgQueue*> g_id2q;
DHandle g_qid = 1;
#define DM_QUITMSG 1000

DVoid* DThreadForQueue(DVoid* pvParam)
{
    g_id2qMutex.lock();
    auto pNodeQueue = g_id2q.find((DHandle)pvParam);
    if (pNodeQueue == g_id2q.end())
    {
        g_id2qMutex.unlock();
        return 0;
    }
    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    g_id2qMutex.unlock();

    DBool bQuit = false;
    while (true)
    {
        if (pq->m_queue.size() == 0)
        {
            pq->m_wait.Reset();
            DEvent::WaitEvent(pq->m_wait, 5000);
        }

        pq->m_queueMutex.lock();
        DQMsg msgNode = pq->m_queue.front();
        pq->m_queue.pop_front();
        pq->m_queueMutex.unlock();

        pq->m_msgfuncMutex.lock();
        auto pFunc = pq->m_msgfunc.begin();
        while (pFunc != pq->m_msgfunc.end())
        {
            if (msgNode.msg == DM_QUITMSG)
            {
                bQuit = true;
                break;
            }

            DMsgFunc func = *pFunc;
            func(msgNode.msg, msgNode.para1, msgNode.para2);

            pFunc++;
        }
        pq->m_msgfuncMutex.unlock();

        if (bQuit)
        {
            break;
        }
    }

    return 0;
}

DMsgQueue::DMsgQueue(DCStr queueName, DUInt32 maxSize)
{
    m_name = queueName;
    m_maxSize = maxSize;
}

DHandle DMsgQueue::Create(DCStr queueName, DUInt32 maxSize)
{
    DMsgQueue* pq = new DMsgQueue(queueName, maxSize);
    pq->m_queue.clear();
    pq->m_msgfunc.clear();
    pq->m_wait.Create(NULL);
    pq->m_wait.Reset();
    pq->m_t.reset(new std::thread(DThreadForQueue, (DVoid*)(g_qid)));
    g_id2qMutex.lock();
    g_id2q.insert(std::pair<DHandle, DMsgQueue*>(g_qid, pq));
    g_id2qMutex.unlock();
    return g_qid++;
}

DHandle DMsgQueue::GetQueue(DCStr queueName)
{
    g_id2qMutex.lock();
    for (auto pNode = g_id2q.begin(); pNode != g_id2q.end(); pNode++) {
        DMsgQueue* pq = (DMsgQueue*)pNode->second;
        if (pq->m_name == queueName) {
            g_id2qMutex.unlock();
            return pNode->first;
        }
    }
    g_id2qMutex.unlock();
    return 0;
}

DVoid DMsgQueue::RemoveQueue(DHandle qid)
{
    g_id2qMutex.lock();
    auto pNodeQueue = g_id2q.find((DHandle)qid);
    if (pNodeQueue == g_id2q.end())
    {
        g_id2qMutex.unlock();
        return;
    }
    
    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    pq->m_t->join();

    pq->m_queue.clear();
    pq->m_msgfunc.clear();
    pq->m_wait.Close();
    delete pq;
    g_id2q.erase((DHandle)qid);
    g_id2qMutex.unlock();
}

DUInt32 DMsgQueue::PostQueueMsg(DHandle qid, DUInt32 msg, DVoid* para1, DVoid* para2)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) {
        return 1;
    }

    DMsgQueue* pQueue = pNodeQueue->second;
    if (pQueue->m_queue.size() >= pQueue->m_maxSize) {
        return 2;
    }

    DQMsg newmsg;
    newmsg.msg = msg;
    newmsg.para1 = para1;
    newmsg.para2 = para2;
    pQueue->m_queueMutex.lock();
    pQueue->m_queue.push_back(newmsg);
    pQueue->m_queueMutex.unlock();
    pQueue->m_wait.Set();
    return 0;
}

DVoid DMsgQueue::PostQuitMsg(DHandle qid)
{
    PostQueueMsg(qid, DM_QUITMSG, 0, 0);
}

DVoid DMsgQueue::Quit(DHandle qid)
{
    // 先清理掉所有的 Handler
    RemoveAllHandler(qid);

    // 再清理掉所有的排队 Msg
    ClearAllMsg(qid);

    // 发一个退出消息
    PostQueueMsg(qid, DM_QUITMSG, 0, 0);

    // 等待线程结束
    RemoveQueue(qid);
}

DBool DMsgQueue::IsInQueue(DHandle qid)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return false;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return false;

    if (pq->m_t->get_id() == std::this_thread::get_id())
    {
        return true;
    }

    return false;
}

DUInt32 DMsgQueue::GetQueueSize(DHandle qid)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return 0;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return 0;

    pq->m_queueMutex.lock();
    DUInt32 nSize = (DUInt32)pq->m_queue.size();
    pq->m_queueMutex.unlock();

    return nSize;
}

DUInt32 DMsgQueue::GetQueueMaxSize(DHandle qid)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return 0;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return 0;

    return pq->m_maxSize;
}

DHandle DMsgQueue::GetCurQueueID()
{
    for (auto pNodeQueue = g_id2q.begin(); pNodeQueue != g_id2q.end(); pNodeQueue++) {
        DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
        if (pq->m_t->get_id() == std::this_thread::get_id())
        {
            return pNodeQueue->first;
        }
    }
    return 0;
}

DCStr DMsgQueue::GetCurQueueName()
{
    for (auto pNodeQueue = g_id2q.begin(); pNodeQueue != g_id2q.end(); pNodeQueue++) {
        DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
        if (pq->m_t->get_id() == std::this_thread::get_id())
        {
            return pq->m_name.c_str();
        }
    }
    return 0;
}

DUInt32 DMsgQueue::GetCoreCount()
{
    return std::thread::hardware_concurrency();
}

DVoid* DMsgQueue::GetThreadHandle(DHandle qid)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return NULL;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return NULL;

    return pq->m_t->native_handle();
}

DVoid DMsgQueue::AddHandler(DHandle qid, DMsgFunc handler)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return;

    pq->m_msgfuncMutex.lock();
    DBool bFind = false;
    for (auto pNode = pq->m_msgfunc.begin(); pNode != pq->m_msgfunc.end(); pNode++) {
        if (*pNode == handler) {
            bFind = true;
        }
    }
    if (!bFind) {
        pq->m_msgfunc.push_back(handler);
    }
    pq->m_msgfuncMutex.unlock();
}

DVoid DMsgQueue::RemoveHandler(DHandle qid, DMsgFunc handler)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return;

    pq->m_msgfuncMutex.lock();
    pq->m_msgfunc.remove(handler);
    pq->m_msgfuncMutex.unlock();
}

DUInt32 DMsgQueue::GetHandlerSize(DHandle qid)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return 0;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return 0;

    pq->m_msgfuncMutex.lock();
    DUInt32 nSize = (DUInt32)pq->m_msgfunc.size();
    pq->m_msgfuncMutex.unlock();

    return nSize;
}

DMsgFunc DMsgQueue::GetHandler(DHandle qid, DUInt32 nIndex)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return 0;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return 0;

    pq->m_msgfuncMutex.lock();
    auto msgfunc = pq->m_msgfunc.begin();
    DUInt32 i = 0;
    while (msgfunc != pq->m_msgfunc.end()) {
        if (i++ == nIndex) {
            break;
        }
        msgfunc++;
    }
    pq->m_msgfuncMutex.unlock();
    return *msgfunc;
}

DVoid DMsgQueue::RemoveAllHandler(DHandle qid)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return;

    pq->m_msgfuncMutex.lock();
    pq->m_msgfunc.clear();
    pq->m_msgfuncMutex.unlock();
}

DVoid DMsgQueue::SetCleaner(DHandle qid, DMsgFunc handler)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return;
    
    pq->m_msgcleaner = handler;
}

DVoid DMsgQueue::ClearAllMsg(DHandle qid)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return;

    pq->m_queueMutex.lock();
    auto msg = pq->m_queue.begin();
    while (msg != pq->m_queue.end()) {
        pq->m_msgcleaner(msg->msg, msg->para1, msg->para2);
        msg++;
    }
    pq->m_queue.clear();
    pq->m_queueMutex.unlock();
}
