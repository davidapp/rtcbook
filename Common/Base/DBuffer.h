#pragma once

#include "DTypes.h"
#include "Base/DAtomic.h"
#include "Base/DList.h"
#include <string>

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
// 
// DBuffer - 管理一块缓存
// DGrowBuffer - 用于组包
// DReadBuffer - 用于解包
// 


struct DBufferData
{
    DAtomInt32 nRefs;
    DInt32 nAllocLength;

    DByte* buf()
    {
        return (DByte*)(this + 1);
    }
};

class DBuffer
{
public:
    DBuffer();
    explicit DBuffer(DUInt32 nSize);
    DBuffer(DVoid* pBuf, DUInt32 size);
    ~DBuffer();
    DVoid Init();

public:
    DBuffer(const DBuffer& buffer);
    DBuffer& operator =(const DBuffer& bufSrc);

public:
    DBool   Reserve(DInt32 nLen);
    DBuffer GetSub(DInt32 start, DInt32 end);
    DVoid   SetSub(DInt32 start, DByte* pBuf, DUInt32 len);

public:
    DUInt32 GetRefCount() const;
    DByte* GetBuf() const;
    DUInt32 GetSize() const;
    DVoid AddRef();
    DVoid Release();
    DVoid Attach(DByte* pBuf);
    DVoid Detach();
    DBool IsNull();

public:
    DVoid  SetAt(DUInt32 index, DByte v);
    DByte  GetAt(DUInt32 index);
    DVoid  Zero();
    DVoid  FillWithRandom();
    DVoid  Xor(DByte mask);

public:
    DInt32 Compare(const DBuffer& buf) const;
    friend inline DBool DX86_STDCALL operator ==(const DBuffer& s1, const DBuffer& s2)
    {
        return (s1.Compare(s2) == 0);
    }

public:
    DByte* LockBuffer();
    DVoid  UnlockBuffer();

protected:
    DByte* m_pBuf;
    DBufferData* GetData() const;

protected:
    DBool AllocBuffer(DInt32 nLen);
    DVoid AssignCopy(DInt32 nSrcLen, DByte* lpszSrcData);
    DBool AllocBeforeWrite(DInt32 nLen);
    DVoid CopyBeforeWrite();

private:
    static DVoid DX86_STDCALL Release(DBufferData* pData);
    const DBuffer& DX86_STDCALL GetNullBuffer();
};


class DGrowBuffer
{
public:
    DGrowBuffer();
    ~DGrowBuffer();

public:
    DVoid AddUInt8(DUInt8 c);       // 1
    DVoid AddUInt16(DUInt16 s, DBool bNetOrder = false);    // 2
    DVoid AddUInt32(DUInt32 i, DBool bNetOrder = false);    // 4
    DVoid AddUInt64(DUInt64 l, DBool bNetOrder = false);    // 8
    DVoid AddFloat(DFloat f);       // 4
    DVoid AddDouble(DDouble d);     // 8
    DVoid AddStringA(std::string& sa);  // len(4)+str
    DVoid AddString(std::wstring& s);     // len(4)+str
    DVoid AddBuffer(DBuffer b);     // len(4)+buf
    DVoid AddFixBuffer(DBuffer b);  // buf
    DBuffer Finish();

private:
    DBuffer m_cache;
    DUInt32 m_cachepos;
    DUInt32 m_totalsize;
    DSList  m_bufList;

private:
    DVoid FlushCacheToList();
};



class DReadBuffer
{
public:
    explicit DReadBuffer(DBuffer b);
    ~DReadBuffer();

public:
    DByte  ReadUInt8();
    DUInt16 ReadUInt16(DBool bNetOrder = false);
    DUInt32 ReadUInt32(DBool bNetOrder = false);
    DUInt64 ReadUInt64(DBool bNetOrder = false);
    DSizeT  ReadPointer();

    DFloat  ReadFloat();
    DDouble ReadDouble();
    std::string ReadStringA();
    std::string ReadFixStringA(int nCount);
    std::wstring ReadString();
    std::wstring ReadFixString(int nWCharCount);
    DBuffer ReadBuffer();
    DBuffer ReadFixBuffer(int nCount);
    DBool   IsFinish();
    DBuffer ReadLine();
    DBuffer ReadToBlank();

public:
    DUInt32 GetCurPos();
    DBuffer GetRemain();
    DUInt32 GetRemainLength();

public:
    DByte  NextUInt8();
    DUInt16 NextUInt16(DBool bNetOrder = false);
    DUInt32 NextUInt32(DBool bNetOrder = false);
    DUInt64 NextUInt64(DBool bNetOrder = false);

private:
    DBuffer m_buf;
    DUInt32 m_curPos;
};
