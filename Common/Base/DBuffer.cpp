#include "DBuffer.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "DXP.h"
#include "DUtil.h"

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


DBool DBuffer::InitWithHexString(DCStr str)
{
    std::string strA(str);
    return InitWithHexString(strA);
}

DBool DBuffer::InitWithHexString(std::string& str)
{
    if (!DBuffer::IsValidHexStr(str.c_str())) return false;
    str = DUtil::replace_str(str, " ", "");
    str = DUtil::replace_str(str, "\r\n", "");
    str = DUtil::replace_str(str, "\r", "");
    str = DUtil::replace_str(str, "\n", "");
    str = DUtil::replace_str(str, "\t", "");
    return HexToBuffer(str.c_str());
}

std::string DBuffer::ToHexString(DUInt32 maxlen)
{
    DChar buf[30] = { 0 };
    std::string str;
    if (IsNull())
    {
        return std::string("nullptr");
    }
    if (maxlen == 0) maxlen = GetSize();
    if (maxlen > GetSize()) maxlen = GetSize();
    str.reserve(maxlen * 2);
    for (unsigned int i = 0; i < maxlen; i++)
    {
        snprintf(buf, 30, "%02X", m_pBuf[i]);
        str += buf;
    }
    return str;
}

std::string DBuffer::ToHexList(DUInt32 width)
{
    std::string strRet;
    char buf[30] = {};
    if (GetData() == _nullBufferData)
    {

    }
    else
    {
        DUInt32 count = 1;
        for (DUInt32 i = 0; i < GetSize(); i++, count++)
        {
            snprintf(buf, 30, "%02X ", m_pBuf[i]);
            strRet += buf;
            if (count % width == 0) strRet += D_LINES;
        }
        strRet += D_LINES;
    }
    return strRet;
}

DBool DBuffer::HexToBuffer(DCStr hexStr)
{
    CopyBeforeWrite();

    DUInt32 len = (DUInt32)strlen(hexStr);
    AllocBuffer(len / 2);
    DInt32 index = 0;
    for (DUInt32 i = 0; i < len; i = i + 2)
    {
        DChar c = hexStr[i];
        if (isdigit(c))
        {
            c = c - '0';
        }
        else if (DUtil::isAtoF(c))
        {
            c = c - 'A' + 10;
        }
        else if (DUtil::isatof(c))
        {
            c = c - 'a' + 10;
        }
        else
        {
            return false;
        }
        DChar d = hexStr[i + 1];
        if (isdigit(d))
        {
            d = d - '0';
        }
        else if (DUtil::isAtoF(d))
        {
            d = d - 'A' + 10;
        }
        else if (DUtil::isatof(d))
        {
            d = d - 'a' + 10;
        }
        else
        {
            return false;
        }
        m_pBuf[index] = c * 16 + d;
        index++;
    }
    return true;
}

DBool DBuffer::IsValidHexStr(DCStr hexStr, DUInt32* reason)
{
    DUInt32 len = (DUInt32)strlen(hexStr);
    if (len == 0)
    {
        if (reason) *reason = 1;
        return false;
    }
    DUInt32 xcount = 0;
    for (int i = 0; i < (int)len; i++)
    {
        if (isxdigit(hexStr[i]))
        {
            xcount++;
        }
        else if (hexStr[i] == ' ' || hexStr[i] == '\r' || hexStr[i] == '\n' || hexStr[i] == '\t')
        {

        }
        else
        {
            if (reason) *reason = 2;
            return false;
        }
    }
    if (xcount % 2 != 0)
    {
        if (reason) *reason = 3;
        return false;
    }
    return true;
}


static DChar base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static DInt32 unbase64[] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52,
    53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, 0, -1, -1, -1,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
    42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1
};

DBool DBuffer::InitWithBase64String(DCStr str)
{
    std::string strA(str);
    return InitWithBase64String(strA);
}

DBool DBuffer::InitWithBase64String(std::string& str)
{
    if (!IsValidBase64Str(str.c_str()))
    {
        return false;
    }
    DByte* input = (DByte*)str.c_str();
    DUInt32 len = (DUInt32)str.length();
    DByte* output = (DByte*)malloc(len);
    DByte* start = output;
    DInt32 out_len = 0;
    do
    {
        for (DInt32 i = 0; i <= 3; i++)
        {
            if (((DByte)str[i]) > 128 || unbase64[input[i]] == -1)
            {
                free(start);
                return false;
            }
        }
        *output++ = unbase64[input[0]] << 2 | (unbase64[input[1]] & 0x30) >> 4;
        out_len++;

        if (input[2] != '=')
        {
            *output++ = (unbase64[input[1]] & 0x0F) << 4 |
                (unbase64[input[2]] & 0x3C) >> 2;
            out_len++;
        }

        if (input[3] != '=')
        {
            *output++ = (unbase64[input[2]] & 0x03) << 6 |
                unbase64[input[3]];
            out_len++;
        }
        input += 4;
    } while (len -= 4);

    AllocBuffer(out_len);
    memcpy_s(m_pBuf, out_len, start, out_len);
    free(start);

    return true;
}

std::string DBuffer::ToBase64String()
{
    const DByte* input = m_pBuf;
    DInt32 len = GetSize();
    DChar* output = (DChar*)malloc((len * 4) / 3 + 4);
    if (output == nullptr) return std::string();
    DChar* begin = output;
    do
    {
        *output++ = base64[(input[0] & 0xFC) >> 2];

        if (len == 1)
        {
            *output++ = base64[((input[0] & 0x03) << 4)];
            *output++ = '=';
            *output++ = '=';
            break;
        }

        *output++ = base64[
            ((input[0] & 0x03) << 4) | ((input[1] & 0xF0) >> 4)];

        if (len == 2)
        {
            *output++ = base64[((input[1] & 0x0F) << 2)];
            *output++ = '=';
            break;
        }

        *output++ = base64[
            ((input[1] & 0x0F) << 2) | ((input[2] & 0xC0) >> 6)];
        *output++ = base64[(input[2] & 0x3F)];
        input += 3;
    } while (len -= 3);

    *output = '\0';

    std::string str(begin);
    free(begin);

    return str;
}

DBool DBuffer::IsValidBase64Str(DCStr base64Str, DUInt32* reason)
{
    DUInt32 len = (DUInt32)strlen(base64Str);
    if (len == 0)
    {
        if (reason) *reason = 1;
        return false;
    }
    DUInt32 xcount = 0;
    DUInt32 ecount = 0;
    for (int i = 0; i < (int)len; i++)
    {
        if (isdigit(base64Str[i]) || isalpha(base64Str[i]) || (base64Str[i] == '+') || (base64Str[i] == '/'))
        {
            xcount++;
        }
        else if (base64Str[i] == ' ' || base64Str[i] == '\r' || base64Str[i] == '\n' || base64Str[i] == '\t')
        {

        }
        else if (base64Str[i] == '=')
        {
            ecount++;
        }
        else
        {
            if (reason) *reason = 2;
            return false;
        }
    }
    DUInt32 totalbit = (xcount + ecount) * 6;
    if (totalbit % 8 == 0)
    {
        return true;
    }

    if (reason) *reason = 3;
    return false;
}

DUInt32 DBuffer::GetBase64BufSize(DCStr base64Str)
{
    DUInt32 len = (DUInt32)strlen(base64Str);
    if (len == 0)
    {
        return 0;
    }
    DUInt32 xcount = 0;
    DUInt32 ecount = 0;
    for (int i = 0; i < (int)len; i++)
    {
        if (isdigit(base64Str[i]) || isalpha(base64Str[i]) || (base64Str[i] == '+') || (base64Str[i] == '/'))
        {
            xcount++;
        }
        else if (base64Str[i] == ' ' || base64Str[i] == '\r' || base64Str[i] == '\n' || base64Str[i] == '\t')
        {

        }
        else if (base64Str[i] == '=')
        {
            ecount++;
        }
        else
        {
            return 0;
        }
    }
    DUInt32 totalbit = (xcount + ecount) * 6;
    if (totalbit % 8 == 0)
    {
        return totalbit / 8 - ecount;
    }
    return 0;
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
            DXP::memcpy(m_pBuf, pData->buf(), pData->nAllocLength);
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
    m_bufList.clear();
}

DGrowBuffer::~DGrowBuffer()
{
    m_cache.Release();
    m_bufList.clear();
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
        s = DUtil::Swap16(s);
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
        i = DUtil::Swap32(i);
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
        l = DUtil::Swap64(l);
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
{
    DInt32 strLen = sa.size();
    if (m_cachepos + strLen + 4 > DGROWBUFFER_SIZE)
    {
        DBuffer buf(strLen + 4);
        buf.SetSub(0, (DByte*)&strLen, 4);
        buf.SetSub(4, (DByte*)sa.c_str(), strLen);
        FlushCacheToList();
        m_bufList.push_back(buf.GetBuf());
        buf.AddRef();
        m_totalsize += buf.GetSize();
        return;
    }

    if (m_cachepos + strLen + 4 <= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
        m_cache.SetSub(0, (DByte*)&strLen, 4);
        m_cache.SetSub(4, (DByte*)sa.c_str(), strLen);
        m_cachepos += strLen + 4;
        return;
    }

    m_cache.SetSub(m_cachepos, (DByte*)&strLen, 4);
    m_cache.SetSub(m_cachepos + 4, (DByte*)sa.c_str(), strLen);
    m_cachepos += strLen + 4;
}

DVoid DGrowBuffer::AddString(std::wstring& s)
{
    DInt32 strLen = s.size() * 2;
    if (m_cachepos + strLen + 4 > DGROWBUFFER_SIZE)
    {
        DBuffer buf(strLen + 4);
        buf.SetSub(0, (DByte*)&strLen, 4);
        buf.SetSub(4, (DByte*)s.c_str(), strLen);
        FlushCacheToList();
        m_bufList.push_back(buf.GetBuf());
        buf.AddRef();
        m_totalsize += buf.GetSize();
        return;
    }

    if (m_cachepos + strLen + 4 <= DGROWBUFFER_SIZE)
    {
        FlushCacheToList();
        m_cache.SetSub(0, (DByte*)&strLen, 4);
        m_cache.SetSub(4, (DByte*)s.c_str(), strLen);
        m_cachepos += strLen + 4;
        return;
    }

    m_cache.SetSub(m_cachepos, (DByte*)&strLen, 4);
    m_cache.SetSub(m_cachepos + 4, (DByte*)s.c_str(), strLen);
    m_cachepos += strLen + 4;
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
        m_bufList.push_back(buf.GetBuf());
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
        m_bufList.push_back(buf.GetBuf());
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
    auto pNode = m_bufList.begin();
    for (; pNode != m_bufList.end(); pNode++)
    {
        DBuffer buf;
        buf.Attach((DByte*)*pNode);
        bufRet.SetSub(curpos, buf.GetBuf(), buf.GetSize());
        curpos += buf.GetSize();
    }
    m_bufList.clear();
    m_totalsize = 0;
    return bufRet;
}

DVoid DGrowBuffer::FlushCacheToList()
{
    if (m_cachepos == 0) return;
    DBuffer buf = m_cache.GetSub(0, m_cachepos);
    m_bufList.push_back(buf.GetBuf());
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
        s = DUtil::Swap16(s);
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
        i = DUtil::Swap32(i);
    }

    return i;
}

DUInt64 DReadBuffer::ReadUInt64(DBool bNetOrder)
{
    DUInt64 i;
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + 8);
    DXP::memcpy(&i, b.GetBuf(), 8);
    m_curPos += 8;

    if (bNetOrder)
    {
        i = DUtil::Swap64(i);
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
        s = DUtil::Swap16(s);
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
        i = DUtil::Swap32(i);
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
        i = DUtil::Swap64(i);
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
{
    DUInt32 strLen = ReadUInt32();
    if (strLen == 0)
    {
        return std::string();
    }
    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + strLen);
    std::string strA((DChar*)b.GetBuf(), (DUInt32)b.GetSize());
    m_curPos += strLen;
    return strA;
}

std::string DReadBuffer::ReadFixStringA(int nCount)
{
    if (m_curPos + nCount > m_buf.GetSize())
    {
        return std::string();
    }

    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + nCount);
    m_curPos += nCount;
    std::string strA((DChar*)b.GetBuf(), b.GetSize());
    return strA;
}

std::wstring DReadBuffer::ReadString()
{
    DUInt32 strLen = ReadUInt32();
    if (strLen == 0)
    {
        return std::wstring();
    }

    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + strLen);
    std::wstring str((DWChar*)b.GetBuf(), (DUInt32)b.GetSize() / 2);
    m_curPos += strLen;
    return str;
}

std::wstring DReadBuffer::ReadFixString(int nWCharCount)
{
    if (m_curPos + nWCharCount * 2 > m_buf.GetSize())
    {
        return std::wstring();
    }

    DBuffer b = m_buf.GetSub(m_curPos, m_curPos + nWCharCount * 2);
    m_curPos += nWCharCount * 2;
    std::wstring str((DWChar*)b.GetBuf(), b.GetSize());
    return str;
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

