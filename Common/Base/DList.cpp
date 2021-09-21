#include "DList.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////////////////////
// DSList
// 双端单链表，维护头尾指针，在Tail处插入快
// [x1]->[x2]->...->[xn]
// ^                 ^
// head             tail


DSList::DSList()
{
    m_pHead = nullptr;
    m_pTail = nullptr;
}

DSList::~DSList()
{
    Clear();
}

DVoid DSList::Clear()
{
    // 从起点开始，依次释放节点
    DSLinkNode* pNode = m_pHead;
    while (pNode != nullptr)
    {
        //临时缓存下个节点
        DSLinkNode* temp = pNode->pNext;
        free(pNode);
        pNode = temp;
    }
    m_pHead = nullptr;
    m_pTail = nullptr;
}

DBool DSList::IsEmpty()
{
    return ((m_pHead == nullptr) && (m_pTail == nullptr));
}

DUInt32 DSList::GetSize()
{
    // 数一下有几个节点
    DUInt32 nCount = 0;
    for (DSLinkNode* pNode = m_pHead; pNode != nullptr; pNode = pNode->pNext)
    {
        nCount++;
    }
    return nCount;
}

DSLinkNode* DSList::GetHead()
{
    return m_pHead;
}

DSLinkNode* DSList::GetTail()
{
    return m_pTail;
}

DVoid DSList::Add(DVoid* pData)
{
    DSList::Add(pData, 0, DSLinkNodeType::Value);
}

DVoid DSList::InsertAfter(DSLinkNode* pNode, DVoid* pData)
{
    DSList::InsertAfter(pNode, pData, 0, DSLinkNodeType::Value);
}

DVoid DSList::Delete(DVoid* pData)
{
    DSLinkNode* pNode = Find(pData);
    if (pNode)
    {
        Delete(pNode);
    }
}

DVoid DSList::Delete(DSLinkNode* pNode)
{
    if (pNode == nullptr) return;

    // 如果只有一个节点，并且要删的就是它
    if ((m_pHead == pNode) && (m_pTail == pNode))
    {
        free(m_pHead);
        m_pHead = nullptr;
        m_pTail = nullptr;
        return;
    }

    // 如果要删除的是 Head
    if (m_pHead == pNode)
    {
        DSLinkNode* pTemp = m_pHead->pNext;
        free(m_pHead);
        m_pHead = pTemp;
    }
    else
    {
        DSLinkNode* pCurNode = m_pHead;
        while (pCurNode != nullptr)
        {
            // 要删除的是下个节点
            if (pCurNode->pNext == pNode)
            {
                pCurNode->pNext = pNode->pNext;
                free(pNode);
                break;
            }
            pCurNode = pCurNode->pNext;
        }

        // 要删除的是最后一个节点
        if (m_pTail == pNode)
        {
            m_pTail = pCurNode;
        }
    }
}

DVoid DSList::Add(DVoid* pData, DUInt32 nDataSize, DSLinkNodeType type)
{
    DSLinkNode* pNode = (DSLinkNode*)malloc(sizeof(DSLinkNode));
    if (pNode)
    {
        pNode->pData = pData;
        pNode->nData32 = nDataSize;
        pNode->nDataType = (DUInt32)type;
        pNode->pNext = nullptr;
        if (m_pHead == nullptr) m_pHead = pNode;
        if (m_pTail == nullptr) m_pTail = pNode;
        else
        {
            m_pTail->pNext = pNode;
            m_pTail = pNode;
        }
    }
}

DVoid DSList::InsertAfter(DSLinkNode* pNode, DVoid* pData, DUInt32 nDataSize, DSLinkNodeType type)
{
    DSLinkNode* pNodeNew = (DSLinkNode*)malloc(sizeof(DSLinkNode));
    if (pNodeNew)
    {
        pNodeNew->pData = pData;
        pNodeNew->pNext = pNode->pNext;
        pNode->pNext = pNodeNew;
    }
}

DSLinkNode* DSList::Find(DVoid* pData)
{
    DSLinkNode* pNode = m_pHead;
    while (pNode != nullptr)
    {
        if (pNode->pData == pData) break;
        pNode = pNode->pNext;
    }
    return pNode;
}

DSLinkNode* DSList::Find32(DUInt32 nData32)
{
    DSLinkNode* pNode = m_pHead;
    while (pNode != nullptr)
    {
        if (pNode->nData32 == nData32) break;
        pNode = pNode->pNext;
    }
    return pNode;
}

DSLinkNode* DSList::FindAfter(DSLinkNode* pNode, DVoid* pData)
{
    DSLinkNode* pCur = pNode;
    while (pCur != nullptr)
    {
        if (pCur->pData == pData) break;
        pCur = pCur->pNext;
    }
    return pCur;
}

DSLinkNode* DSList::Find32After(DUInt32 nData32)
{
    DSLinkNode* pNode = m_pHead;
    while (pNode != nullptr)
    {
        if (pNode->nData32 == nData32) break;
        pNode = pNode->pNext;
    }
    return pNode;
}
