#include "DMsgQueue.h"
#include <map>

std::mutex g_id2qMutex;
std::map<DHandle, DMsgQueue*> g_id2q;
DHandle g_qid = 1;

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
            DEvent::WaitEvent(pq->m_wait, D_INFINITE);
        }

        pq->m_queueMutex.lock();
        DQMsg msgNode = pq->m_queue.front();
        pq->m_queueMutex.unlock();

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

        pq->m_queueMutex.lock();
        pq->m_queue.pop_front();
        pq->m_queueMutex.unlock();

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
    g_id2q.insert(std::pair<DHandle, DMsgQueue*>(g_qid, pq));
    return g_qid++;
}

DHandle DMsgQueue::GetQueue(DCStr queueName)
{
    for (auto pNode = g_id2q.begin(); pNode != g_id2q.end(); pNode++) {
        DMsgQueue* pq = (DMsgQueue*)pNode->second;
        if (pq->m_name == queueName) {
            return pNode->first;
        }
    }
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

DUInt32 DMsgQueue::SendQueueMsg(DHandle qid, DUInt32 msg, DVoid* para1, DVoid* para2)
{
    return 0;
}

DVoid DMsgQueue::PostQuitMsg(DHandle qid)
{
    PostQueueMsg(qid, DM_QUITMSG, 0, 0);
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

    DBool bFind = false;
    for (auto pNode = pq->m_msgfunc.begin(); pNode != pq->m_msgfunc.end(); pNode++) {
        if (*pNode == handler) {
            bFind = true;
        }
    }

    if (!bFind) {
        pq->m_msgfunc.push_back(handler);
    }
}

DVoid DMsgQueue::RemoveHandler(DHandle qid, DMsgFunc handler)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return;

    pq->m_msgfunc.remove(handler);
}

DVoid DMsgQueue::RemoveAllHandler(DHandle qid)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return;

    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->second;
    if (pq == NULL) return;

    pq->m_msgfunc.clear();
}
