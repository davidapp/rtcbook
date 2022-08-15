#include "DMsgQueue.h"
#include <map>
#include <thread>

std::mutex g_id2qMutex;
std::map<DHandle, DMsgQueue*> g_id2q;
DHandle g_qid = 1;

DVoid* DThreadForQueue(DVoid* pvParam)
{
    /*g_id2qMutex.Lock();
    DRBTreeNode* pNodeQueue = g_id2q.Find(pvParam);
    if (pNodeQueue == NULL)
    {
        g_id2qMutex.Unlock();
        return 0;
    }
    DMsgQueue* pq = (DMsgQueue*)pNodeQueue->user_data;
    g_id2qMutex.Unlock();

    DBool bQuit = false;
    while (true)
    {
        if (pq->m_queue.GetSize() == 0)
        {
            pq->m_wait.Reset();
            DThread::WaitEvent(pq->m_wait, D_INFINITE);
        }

        pq->m_queueMutex.Lock();
        DSLinkNode* pMsgNode = pq->m_queue.GetHead();
        if (pMsgNode == NULL) break;
        pq->m_queueMutex.Unlock();

        DSLinkNode* pNode = pq->m_msgfunc.GetHead();
        while (pNode != NULL)
        {
            if (((DQMsg*)(pMsgNode->pData))->msg == DM_QUITMSG)
            {
                pq->m_queueMutex.Lock();
                pq->m_queue.Delete(pMsgNode);
                pq->m_queueMutex.Unlock();
                //delete pMsgNode->pData;
                bQuit = true;
                break;
            }

            DMsgFunc func = (DMsgFunc)pNode->pData;
            func(((DQMsg*)(pMsgNode->pData))->msg, ((DQMsg*)(pMsgNode->pData))->para1, ((DQMsg*)(pMsgNode->pData))->para2);

            delete (DQMsg*)(pMsgNode->pData);
            pq->m_queueMutex.Lock();
            pq->m_queue.Delete(pMsgNode);
            pq->m_queueMutex.Unlock();

            pNode = pNode->pNext;
        }

        if (bQuit)
        {
            break;
        }
    }

    g_id2qMutex.Lock();
    g_id2q.Erase(pNodeQueue);
    g_id2qMutex.Unlock();

    delete pq;
    DPrintf("DMsgQueue Quit.\n");
    return 0;*/
    return 0;
}

DVoid DMsgQueue::Init()
{
    
}

DVoid DMsgQueue::Destroy()
{

}

DHandle DMsgQueue::Create(DCStr queueName, DUInt32 maxSize)
{
    DMsgQueue* pq = new DMsgQueue();
    pq->m_name = queueName;
    pq->m_queue.clear();
    pq->m_msgfunc.clear();
    pq->m_wait.Reset();
    pq->maxSize = maxSize;

    g_id2q.insert(std::pair<DHandle, DMsgQueue*>(g_qid, pq));

    std::thread t = std::thread(DThreadForQueue, (DVoid*)(g_qid));
    t.detach();

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

DBool DMsgQueue::PostQueueMsg(DHandle qid, DUInt32 msg, DVoid* para1, DVoid* para2)
{
    auto pNodeQueue = g_id2q.find(qid);
    if (pNodeQueue == g_id2q.end()) return false;

    DMsgQueue* pQueue = pNodeQueue->second;
    if (pQueue->m_queue.size() > pQueue->maxSize) {
        return false;
    }

    DQMsg newmsg;
    newmsg.msg = msg;
    newmsg.para1 = para1;
    newmsg.para2 = para2;
    pQueue->m_queueMutex.lock();
    pQueue->m_queue.push_back(newmsg);
    pQueue->m_queueMutex.unlock();
    pQueue->m_wait.Set();
    return true;
}

DBool DMsgQueue::SendQueueMsg(DHandle qid, DUInt32 msg, DVoid* para1, DVoid* para2)
{
    return true;
}

DVoid DMsgQueue::PostQuitMsg(DHandle qid)
{
    PostQueueMsg(qid, DM_QUITMSG, 0, 0);
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
