#pragma once

#include "DTypes.h"

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
// 
// DSList - 双端单链表，维护头尾指针
// 


enum class DSLinkNodeType
{
    Value = 0,
    Buffer_Free = 1,
    Object_Delete = 2,
};

typedef struct tagDSLinkNode
{
    struct tagDSLinkNode* pNext;
    DVoid* pData;
    DUInt32 nData32;
    DUInt32 nDataType;
} DSLinkNode;


class DSList
{
public:
    DSList();
    ~DSList();
    DVoid Clear();
    DBool IsEmpty();
    DUInt32 GetSize();
    DSLinkNode* GetHead();
    DSLinkNode* GetTail();

public:
    DVoid Add(DVoid* pData);
    DVoid InsertAfter(DSLinkNode* pNode, DVoid* pData);
    DVoid Delete(DVoid* pData);
    DVoid Delete(DSLinkNode* pNode);
    DSLinkNode* Find(DVoid* pData);
    DSLinkNode* Find32(DUInt32 nData32);
    DSLinkNode* FindAfter(DSLinkNode* pNode, DVoid* pData);
    DSLinkNode* Find32After(DUInt32 nData32);

    DVoid Add(DVoid* pData, DUInt32 nDataSize, DSLinkNodeType type);
    DVoid InsertAfter(DSLinkNode* pNode, DVoid* pData, DUInt32 nDataSize, DSLinkNodeType type);
    
    D_DISALLOW_COPY_AND_ASSIGN(DSList)

private:
    DSLinkNode* m_pHead;
    DSLinkNode* m_pTail;
};
