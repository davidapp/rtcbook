#include "DBuffer.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "Net/DNet.h"

////////////////////////////////////////////////////////////////////////////////////
// DBuffer

const DInt32 nNullBuffer[] = { -1, 0, 0, 0 };
const DBufferData* _nullBufferData = (DBufferData*)&nNullBuffer;
const DByte* _nullBuffer = (const DByte*)(((DChar*)&nNullBuffer) + sizeof(DBufferData));

DBuffer::DBuffer()
{
    Init();
}

DBuffer::DBuffer(DUInt32 nSize)
{
    Init();
    AllocBuffer((DInt32)nSize);
    Zero();
}

DBuffer::DBuffer(DVoid* pBuf, DUInt32 size)
{
    Init();
    AllocBuffer(size);
    memcpy(m_pBuf, pBuf, size);
}

DBuffer::~DBuffer()
{
    if (GetData() != _nullBufferData)
    {
        DAtomDec32(&GetData()->nRefs);
        if (GetData()->nRefs <= 0)
        {
            free((DVoidPtr)GetData());
            Init();
        }
    }
}

DVoid DBuffer::Init()
{
    m_pBuf = GetNullBuffer().m_pBuf;
}


DBuffer::DBuffer(const DBuffer& buffer)
{
    assert(buffer.GetData()->nRefs != 0);

    if (buffer.GetData()->nRefs > 0)
    {
        assert(buffer.GetData() != _nullBufferData);
        m_pBuf = buffer.m_pBuf;
        DAtomInc32(&GetData()->nRefs);
    }
    else
    {
        Init();
        AllocBuffer(buffer.GetSize());
        memcpy(m_pBuf, buffer.GetBuf(), buffer.GetSize());
    }
}

DBuffer& DBuffer::operator =(const DBuffer& bufSrc)
{
    if (m_pBuf != bufSrc.m_pBuf)
    {
        if ((GetData()->nRefs < 0 && GetData() != _nullBufferData) || bufSrc.GetData()->nRefs < 0)
        {
            if (AllocBeforeWrite(bufSrc.GetSize()))
            {
                memcpy(m_pBuf, bufSrc.GetBuf(), bufSrc.GetSize());
            }
        }
        else
        {
            Release();
            assert(bufSrc.GetData() != _nullBufferData);
            m_pBuf = bufSrc.m_pBuf;
            DAtomInc32(&GetData()->nRefs);
        }
    }
    return *this;
}


DBool DBuffer::Reserve(DInt32 charCount)
{
    return AllocBuffer(charCount);
}

DBuffer DBuffer::GetSub(DInt32 start, DInt32 end)
{
    assert(start >= 0 && start <= GetData()->nAllocLength);
    assert(end >= start && end <= GetData()->nAllocLength);

    DBuffer bufRet(m_pBuf + start, end - start);
    return bufRet;
}

DVoid DBuffer::SetSub(DInt32 start, DByte* pBuf, DUInt32 len)
{
    assert(start >= 0 && start <= GetData()->nAllocLength);
    assert(start + len <= (DUInt32)(GetData()->nAllocLength));

    memcpy(m_pBuf + start, pBuf, len);
}

DUInt32 DBuffer::GetRefCount() const
{
    return (DUInt32)(GetData()->nRefs);
}

DByte* DBuffer::GetBuf() const
{
    return m_pBuf;
}

DUInt32 DBuffer::GetSize() const
{
    return GetData()->nAllocLength;
}

DVoid DBuffer::AddRef()
{
    DAtomInc32(&(GetData()->nRefs));
}

DVoid DBuffer::Release()
{
    if (GetData() != _nullBufferData)
    {
        assert(GetData()->nRefs != 0);
        DAtomDec32(&GetData()->nRefs);
        if (GetData()->nRefs <= 0)
        {
            free((void*)GetData());
        }
        Init();
    }
}

DVoid DBuffer::Attach(DByte* pBuf)
{
    assert(m_pBuf == _nullBuffer);
    m_pBuf = pBuf;
}

DVoid DBuffer::Detach()
{
    Init();
}

DBool DBuffer::IsNull()
{
    return (m_pBuf == GetNullBuffer().m_pBuf);
}

DVoid DBuffer::SetAt(DUInt32 index, DByte v)
{
    CopyBeforeWrite();
    assert(index < GetSize());
    m_pBuf[index] = v;
}

DByte DBuffer::GetAt(DUInt32 index)
{
    assert(index < GetSize());
    return m_pBuf[index];
}

DVoid DBuffer::Zero()
{
    CopyBeforeWrite();
    memset(m_pBuf, 0, GetSize());
}

DVoid DBuffer::FillWithRandom()
{
    CopyBeforeWrite();
    for (DUInt32 i = 0; i < GetSize(); i++)
    {
        m_pBuf[i] = (DChar)rand();
    }
}

DVoid DBuffer::Xor(DByte mask)
{
    DByte* p = GetBuf();
    for (DUInt32 i = 0; i < GetSize(); i++)
    {
        *p = ~(*p) ^ mask;
        p++;
    }
}

DInt32 DBuffer::Compare(const DBuffer& buf) const
{
    if (buf.GetSize() > GetSize()) return -1;
    else if (buf.GetSize() < GetSize()) return 1;
    else {
        DInt32 res = memcmp(m_pBuf, buf.GetBuf(), GetSize());
        return res;
    }
}

DByte* DBuffer::LockBuffer()
{
    if (m_pBuf != nullptr)
    {
        GetData()->nRefs = -1;
    }
    return m_pBuf;
}

DVoid DBuffer::UnlockBuffer()
{
    assert(GetData()->nRefs == -1);
    if (GetData() != _nullBufferData)
    {
        GetData()->nRefs = 1;
    }
}


DBufferData* DBuffer::GetData() const
{
    assert(m_pBuf != nullptr);
    return ((DBufferData*)m_pBuf) - 1;
}

DBool DBuffer::AllocBuffer(DInt32 nLen)
{
    assert(nLen >= 0);

    if (GetData()->nRefs >= 1)
    {
        Release();
    }

    if (nLen == 0)
    {
        Init();
    }
    else
    {
        DBufferData* pData = (DBufferData*)malloc(sizeof(DBufferData) + nLen);
        if (pData == nullptr)
            return false;

        pData->nRefs = 1;
        pData->nAllocLength = nLen;
        m_pBuf = pData->buf();
    }

    return true;
}

DVoid DBuffer::AssignCopy(DInt32 nSrcLen, DByte* lpszSrcData)
{
    if (AllocBeforeWrite(nSrcLen))
    {
        memcpy(m_pBuf, lpszSrcData, nSrcLen);
        GetData()->nAllocLength = nSrcLen;
    }
}

DBool DBuffer::AllocBeforeWrite(DInt32 nLen)
{
    DBool bRet = true;
    if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
    {
        Release();
        bRet = AllocBuffer(nLen);
    }
    assert(GetData()->nRefs <= 1);
    return bRet;
}

DVoid DBuffer::CopyBeforeWrite()
{
    if (GetData()->nRefs > 1)
    {
        DBufferData* pData = GetData();
        Release();
        if (AllocBuffer(pData->nAllocLength))
        {
            memcpy(m_pBuf, pData->buf(), pData->nAllocLength);
        }
    }
    assert(GetData()->nRefs <= 1);
}


DVoid DX86_STDCALL DBuffer::Release(DBufferData* pData)
{
    if (pData != _nullBufferData)
    {
        assert(pData->nRefs != 0);
        DAtomDec32(&pData->nRefs);
        if (pData->nRefs <= 0)
        {
            free((void*)pData);
        }
    }
}

const DBuffer& DX86_STDCALL DBuffer::GetNullBuffer()
{
    return *(DBuffer*)&_nullBuffer;
}


////////////////////////////////////////////////////////////////////////////////////
// DGrowBuffer

#define DGROWBUFFER_SIZE 128

DGrowBuffer::DGrowBuffer()
{
    m_cache.Reserve(DGROWBUFFER_SIZE);
    m_cache.Zero();
    m_cachepos = 0;
    m_totalsize = 0;
    m_bufList.Clear();
}

DGrowBuffer::~DGrowBuffer()
{
    m_cache.Release();
    m_bufList.Clear();
}

DVoid DGrowBuffer::AddUInt8(DUInt8 c)
{
    if (m_cachepos + 1 >= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
    }
    m_cache.SetAt(m_cachepos, c);
    m_cachepos++;
}

DVoid DGrowBuffer::AddUInt16(DUInt16 s, DBool bNetOrder)
{
    if (m_cachepos + 2 >= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
    }
    if (bNetOrder)
    {
        s = DNet::Swap16(s);
    }
    m_cache.SetSub(m_cachepos, (DByte*)&s, 2);
    m_cachepos += 2;
}

DVoid DGrowBuffer::AddUInt32(DUInt32 i, DBool bNetOrder)
{
    if (m_cachepos + 4 >= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
    }
    if (bNetOrder)
    {
        i = DNet::Swap32(i);
    }
    m_cache.SetSub(m_cachepos, (DByte*)&i, 4);
    m_cachepos += 4;
}

DVoid DGrowBuffer::AddUInt64(DUInt64 l, DBool bNetOrder)
{
    if (m_cachepos + 8 >= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
    }
    if (bNetOrder)
    {
        l = DNet::Swap64(l);
    }
    m_cache.SetSub(m_cachepos, (DByte*)&l, 8);
    m_cachepos += 8;
}

DVoid DGrowBuffer::AddFloat(DFloat f)
{
    if (m_cachepos + 4 >= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
    }
    m_cache.SetSub(m_cachepos, (DByte*)&f, 4);
    m_cachepos += 4;
}

DVoid DGrowBuffer::AddDouble(DDouble d)
{
    if (m_cachepos + 8 >= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
    }
    m_cache.SetSub(m_cachepos, (DByte*)&d, 8);
    m_cachepos += 8;
}

DVoid DGrowBuffer::AddStringA(std::string& sa)
{/*
    DInt32 strLen = sa.GetDataLength();
    if (m_cachepos + strLen + 4 > DGROWBUFFER_SIZE)
    {
        DBuffer buf(strLen + 4);
        buf.SetSub(0, (DByte*)&strLen, 4);
        buf.SetSub(4, (DByte*)sa.GetStr(), strLen);
        FlushCacheToList();
        m_bufList.Add(buf.GetBuf());
        buf.AddRef();
        m_totalsize += buf.GetSize();
        return;
    }

    if (m_cachepos + strLen + 4 <= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
        m_cache.SetSub(0, (DByte*)&strLen, 4);
        m_cache.SetSub(4, (DByte*)sa.GetStr(), strLen);
        m_cachepos += strLen + 4;
        return;
    }

    m_cache.SetSub(m_cachepos, (DByte*)&strLen, 4);
    m_cache.SetSub(m_cachepos + 4, (DByte*)sa.GetStr(), strLen);
    m_cachepos += strLen + 4;
    */
}

DVoid DGrowBuffer::AddString(std::wstring& s)
{/*
    DInt32 strLen = s.GetDataLength() * 2;
    if (m_cachepos + strLen + 4 > DGROWBUFFER_SIZE)
    {
        DBuffer buf(strLen + 4);
        buf.SetSub(0, (DByte*)&strLen, 4);
        buf.SetSub(4, (DByte*)s.GetStr(), strLen);
        FlushCacheToList();
        m_bufList.Add(buf.GetBuf());
        buf.AddRef();
        m_totalsize += buf.GetSize();
        return;
    }

    if (m_cachepos + strLen + 4 <= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
        m_cache.SetSub(0, (DByte*)&strLen, 4);
        m_cache.SetSub(4, (DByte*)s.GetStr(), strLen);
        m_cachepos += strLen + 4;
        return;
    }

    m_cache.SetSub(m_cachepos, (DByte*)&strLen, 4);
    m_cache.SetSub(m_cachepos + 4, (DByte*)s.GetStr(), strLen);
    m_cachepos += strLen + 4;
    */
}

DVoid DGrowBuffer::AddBuffer(DBuffer b)
{
    DInt32 strLen = b.GetSize();
    if (m_cachepos + strLen + 4 > DGROWBUFFER_SIZE)
    {
        DBuffer buf(strLen + 4);
        buf.SetSub(0, (DByte*)&(strLen), 4);
        buf.SetSub(4, (DByte*)b.GetBuf(), strLen);
        FlushCacheToList();
        m_bufList.Add(buf.GetBuf());
        buf.AddRef();
        m_totalsize += buf.GetSize();
        return;
    }

    if (m_cachepos + strLen + 4 <= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
        m_cache.SetSub(0, (DByte*)&strLen, 4);
        m_cache.SetSub(4, (DByte*)b.GetBuf(), strLen);
        m_cachepos += strLen + 4;
        return;
    }

    m_cache.SetSub(m_cachepos, (DByte*)&strLen, 4);
    m_cache.SetSub(m_cachepos + 4, (DByte*)b.GetBuf(), strLen);
    m_cachepos += strLen + 4;
}

DVoid DGrowBuffer::AddFixBuffer(DBuffer b)
{
    DInt32 strLen = b.GetSize();
    if (m_cachepos + strLen > DGROWBUFFER_SIZE)
    {
        DBuffer buf(strLen);
        buf.SetSub(0, (DByte*)b.GetBuf(), strLen);
        FlushCacheToList();
        m_bufList.Add(buf.GetBuf());
        buf.AddRef();
        m_totalsize += buf.GetSize();
        return;
    }

    if (m_cachepos + strLen + 4 <= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
        m_cache.SetSub(0, (DByte*)b.GetBuf(), strLen);
        m_cachepos += strLen;
        return;
    }

    m_cache.SetSub(m_cachepos, (DByte*)b.GetBuf(), strLen);
    m_cachepos += strLen;
}

DBuffer DGrowBuffer::Finish()
{
    FlushCacheToList();
    DBuffer bufRet(m_totalsize);
    DInt32 curpos = 0;
    DSLinkNode* pNode = m_bufList.GetHead();
    for (; pNode != nullptr; pNode = pNode->pNext)
    {
        DBuffer buf;
        buf.Attach((DByte*)pNode->pData);
        bufRet.SetSub(curpos, buf.GetBuf(), buf.GetSize());
        curpos += buf.GetSize();
    }
    m_bufList.Clear();
    m_totalsize = 0;
    return bufRet;
}

DVoid DGrowBuffer::FlushCacheToList()
{
    if (m_cachepos == 0) return;
    DBuffer buf = m_cache.GetSub(0, m_cachepos);
    m_bufList.Add(buf.GetBuf());
    buf.AddRef();
    m_totalsize += buf.GetSize();
    m_cachepos = 0;
}


////////////////////////////////////////////////////////////////////////////////////
// DReadBuffer

DReadBuffer::DReadBuffer(DBuffer b)
{
    m_buf = b;
    m_curPos = 0;
}

DReadBuffer::~DReadBuffer()
{

}

DByte DReadBuffer::ReadUInt8()
{
    DByte c = m_buf.GetAt(m_curPos);
    m_curPos++;
    return c;
}

DUInt16 DReadBuffer::ReadUInt16(DBool bNetOrder)
{
    DUInt16 s;
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + 2);
    memcpy(&s, b.GetBuf(), 2);
    m_curPos += 2;

    if (bNetOrder)
    {
        s = DNet::Swap16(s);
    }

    return s;
}

DUInt32 DReadBuffer::ReadUInt32(DBool bNetOrder)
{
    DUInt32 i;
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + 4);
    memcpy(&i, b.GetBuf(), 4);
    m_curPos += 4;

    if (bNetOrder)
    {
        i = DNet::Swap32(i);
    }

    return i;
}

DUInt64 DReadBuffer::ReadUInt64(DBool bNetOrder)
{
    DUInt64 i;
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + 8);
    memcpy(&i, b.GetBuf(), 8);
    m_curPos += 8;

    if (bNetOrder)
    {
        i = DNet::Swap64(i);
    }

    return i;
}

DSizeT DReadBuffer::ReadPointer()
{
#if defined(TARGET_64BIT) && (TARGET_64BIT==1)
    return ReadUInt64();
#else 
    return ReadUInt32();
#endif
}

DBool DReadBuffer::IsFinish()
{
    if (m_curPos == m_buf.GetSize()) return true;
    return false;
}

DByte  DReadBuffer::NextUInt8()
{
    DByte c = m_buf.GetAt(m_curPos);
    return c;
}

DUInt16 DReadBuffer::NextUInt16(DBool bNetOrder)
{
    DUInt16 s;
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + 2);
    memcpy(&s, b.GetBuf(), 2);

    if (bNetOrder)
    {
        s = DNet::Swap16(s);
    }

    return s;
}

DUInt32 DReadBuffer::NextUInt32(DBool bNetOrder)
{
    DUInt32 i;
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + 4);
    memcpy(&i, b.GetBuf(), 4);

    if (bNetOrder)
    {
        i = DNet::Swap32(i);
    }

    return i;
}

DUInt64 DReadBuffer::NextUInt64(DBool bNetOrder)
{
    DUInt64 i;
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + 8);
    memcpy(&i, b.GetBuf(), 8);

    if (bNetOrder)
    {
        i = DNet::Swap64(i);
    }

    return i;
}

DFloat DReadBuffer::ReadFloat()
{
    DFloat f;
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + 4);
    memcpy(&f, b.GetBuf(), 4);
    m_curPos += 4;
    return f;
}

DDouble DReadBuffer::ReadDouble()
{
    DDouble d;
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + 8);
    memcpy(&d, b.GetBuf(), 8);
    m_curPos += 8;
    return d;
}

std::string DReadBuffer::ReadStringA()
{/*
    DUInt32 strLen = ReadUInt32();
    if (strLen == 0)
    {
        return DStringA();
    }
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + strLen);
    DStringA strA((DChar*)b.GetBuf(), (DUInt32)b.GetSize());
    m_curPos += strLen;
    return strA;*/
    return std::string();
}

std::string DReadBuffer::ReadFixStringA(int nCount)
{
    /*
    if (m_curPos + nCount > m_buf.GetSize())
    {
        return DStringA();
    }

    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + nCount);
    m_curPos += nCount;
    DStringA strA((DChar*)b.GetBuf(), b.GetSize());
    return strA;
    */
    return std::string();
}

std::wstring DReadBuffer::ReadString()
{
    /*
    DUInt32 strLen = ReadUInt32();
    if (strLen == 0)
    {
        return DString();
    }
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + strLen);
    DString str((DWChar*)b.GetBuf(), (DUInt32)b.GetSize() / 2);
    m_curPos += strLen;
    return str;
    */
    return std::wstring();
}

std::wstring DReadBuffer::ReadFixString(int nWCharCount)
{
    /*
    if (m_curPos + nWCharCount * 2 > m_buf.GetSize())
    {
        return DString();
    }

    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + nWCharCount * 2);
    m_curPos += nWCharCount * 2;
    DString str((DWChar*)b.GetBuf(), b.GetSize());
    return str;
    */
    return std::wstring();
}

DBuffer DReadBuffer::ReadBuffer()
{
    DUInt32 strLen = ReadUInt32();
    if (strLen == 0)
    {
        return DBuffer();
    }
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + strLen);
    m_curPos += strLen;
    return b;
}

DBuffer DReadBuffer::ReadFixBuffer(int nCount)
{
    if (m_curPos + nCount > m_buf.GetSize())
    {
        return DBuffer();
    }

    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + nCount);
    m_curPos += nCount;
    return b;
}

DBuffer DReadBuffer::ReadLine()
{
    DBuffer bufRet;
    if (m_curPos >= m_buf.GetSize() - 1)
    {
        return bufRet;
    }

    DByte* pBuf = m_buf.GetBuf();
    for (DUInt32 i = m_curPos; i < m_buf.GetSize(); i++)
    {
        if (pBuf[i] == 0x0D && pBuf[i + 1] == 0x0A)
        {
            bufRet = m_buf.GetSub(m_curPos, i + 2);
            m_curPos = i + 2;
            break;
        }
    }
    return bufRet;
}

DBuffer DReadBuffer::ReadToBlank()
{
    DBuffer buf;
    return buf;
}

DUInt32 DReadBuffer::GetCurPos()
{
    return m_curPos;
}

DBuffer DReadBuffer::GetRemain()
{
    DBuffer b = m_buf.GetSub(m_curPos, m_buf.GetSize() - m_curPos);
    return b;
}

DUInt32 DReadBuffer::GetRemainLength()
{
    return m_buf.GetSize() - m_curPos;
}

