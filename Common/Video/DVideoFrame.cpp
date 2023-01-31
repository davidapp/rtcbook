#include "DVideoFrame.h"
#include <assert.h>
#include "Base/DXP.h"
#include "Video/DYUV.h"

////////////////////////////////////////////////////////////////////////////////////
// DVideoFrame

struct DVideoFrameMemory{
    DVideoFrameData data;
    DVoid* buf;
};
const DVideoFrameMemory nNullVideoFrame = { {-1, 0, 0, 0, DPixelFmt::Unknown, DMemType::RAW,0,0,0,0}, nullptr };
const DVideoFrameData* _nullVideoFrameData = & nNullVideoFrame.data;
const DByte* _nullVideoFrame = (const DByte*)_nullVideoFrameData + sizeof(DVideoFrameData);


DVideoFrame::DVideoFrame()
{
    Init();
}

DVideoFrame::DVideoFrame(DInt32 w, DInt32 h, DPixelFmt fmt)
{
    Init();
    AllocFrame(w, h, fmt);
    Zero();
}

DVideoFrame::DVideoFrame(DByte* data, DUInt32 data_size, DInt32 w, DInt32 h, DPixelFmt fmt, DMemType mtype)
{
    Init();
    AllocFrame(data_size, w, h, DVideoFrame::DefaultStride(w,fmt), fmt, mtype);
    DXP::memcpy(m_pBuf, data, data_size);
}

DVideoFrame::~DVideoFrame()
{
    if (GetData() != _nullVideoFrameData)
    {
        DAtomDec32(&GetData()->nRefs);
        if (GetData()->nRefs <= 0)
        {
            free((DVoidPtr)GetData());
            Init();
        }
    }
}

DVoid DVideoFrame::Init()
{
    m_pBuf = GetNullVideoFrame().m_pBuf;
}

DVideoFrame::DVideoFrame(const DVideoFrame& frame) 
{
    assert(frame.GetData()->nRefs != 0);

    if (frame.GetData()->nRefs > 0)
    {
        assert(frame.GetData() != _nullVideoFrameData);
        m_pBuf = frame.m_pBuf;
        DAtomInc32(&GetData()->nRefs);
    }
    else
    {
        Init();
        AllocFrame(frame.GetSize(), frame.GetWidth(), frame.GetHeight(), 
            frame.GetLineSize(),frame.GetFormat(), frame.GetMemType());
        memcpy(m_pBuf, frame.GetBuf(), frame.GetSize());
    }
}

DVideoFrame& DVideoFrame::operator =(const DVideoFrame& bufSrc)
{
    if (m_pBuf != bufSrc.m_pBuf)
    {
        if ((GetData()->nRefs < 0 && GetData() != _nullVideoFrameData) || bufSrc.GetData()->nRefs < 0)
        {
            if (AllocBeforeWrite(bufSrc))
            {
                memcpy(m_pBuf, bufSrc.GetBuf(), bufSrc.GetSize());
            }
        }
        else
        {
            Release();
            assert(bufSrc.GetData() != _nullVideoFrameData);
            m_pBuf = bufSrc.m_pBuf;
            DAtomInc32(&GetData()->nRefs);
        }
    }
    return *this;
}

DInt32 DVideoFrame::GetWidth() const
{
    return GetData()->m_width;
}

DInt32 DVideoFrame::GetHeight() const
{
    return GetData()->m_height;
}

DPixelFmt DVideoFrame::GetFormat() const
{
    return GetData()->m_fmt;
}

DMemType DVideoFrame::GetMemType() const
{
    return GetData()->m_type;
}

DInt32 DVideoFrame::GetLineSize() const
{
    return GetData()->m_stride;
}

DUInt64 DVideoFrame::GetCTS() const
{
    return GetData()->m_cts;
}

DVoid DVideoFrame::SetCTS(DUInt64 ts)
{
    GetData()->m_cts = ts;
}

DUInt64 DVideoFrame::GetDTS() const
{
    return GetData()->m_dts;
}

DVoid DVideoFrame::SetDTS(DUInt64 ts)
{
    GetData()->m_dts = ts;
}

DUInt64 DVideoFrame::GetPTS() const
{
    return GetData()->m_pts;
}

DVoid DVideoFrame::SetPTS(DUInt64 ts)
{
    GetData()->m_pts = ts;
}

DUInt32 DVideoFrame::GetRefCount() const
{
    return (DUInt32)(GetData()->nRefs);
}

DByte* DVideoFrame::GetBuf() const
{
    return m_pBuf;
}

DUInt32 DVideoFrame::GetSize() const
{
    return GetData()->nAllocLength;
}

DVoid DVideoFrame::AddRef()
{
    DAtomInc32(&(GetData()->nRefs));
}

DVoid DVideoFrame::Release()
{
    if (GetData() != _nullVideoFrameData)
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

DVoid DVideoFrame::Attach(DByte* pBuf)
{
    assert(m_pBuf == _nullVideoFrame);
    m_pBuf = pBuf;
}

DVoid DVideoFrame::Detach()
{
    Init();
}

DBool DVideoFrame::IsNull() const
{
    return (m_pBuf == GetNullVideoFrame().m_pBuf);
}

DVoid DVideoFrame::SetAt(DUInt32 index, DByte v)
{
    CopyBeforeWrite();
    assert(index < GetSize());
    m_pBuf[index] = v;
}

DByte DVideoFrame::GetAt(DUInt32 index)
{
    assert(index < GetSize());
    return m_pBuf[index];
}

DVoid DVideoFrame::Zero()
{
    CopyBeforeWrite();
    memset(m_pBuf, 0, GetSize());
}


DInt32 DVideoFrame::DefaultStride(DInt32 width, DPixelFmt fmt) 
{
    switch (fmt)
    {
    case DPixelFmt::I420:
        return width;
    case DPixelFmt::RGB565:
    case DPixelFmt::YUY2:
        return width * 2;
    case DPixelFmt::RGB24:
        return width * 3;
    case DPixelFmt::BGRA:
    case DPixelFmt::ARGB:
        return width * 4;
    default:
        return 0;
    }
}

DVideoFrame DVideoFrame::YUY2ToRAW(const DVideoFrame& srcFrame)
{
    DBuffer bufRGB(srcFrame.GetWidth() * srcFrame.GetHeight() * 3);
    DByte* pSRC = srcFrame.GetBuf();
    DByte* pEnd = pSRC + srcFrame.GetSize();
    DByte* pDst = bufRGB.GetBuf();
    while (pSRC != pEnd)
    {
        DYUV::YUV2RGB((DUInt8*)pDst, pSRC[0], pSRC[1] - 128, pSRC[3] - 128);
        pDst += 3;
        DYUV::YUV2RGB((DUInt8*)pDst, pSRC[2], pSRC[1] - 128, pSRC[3] - 128);
        pDst += 3;
        pSRC += 4;
    }
    DVideoFrame retFrame(bufRGB.GetBuf(), bufRGB.GetSize(), srcFrame.GetWidth(), srcFrame.GetHeight(),
                DPixelFmt::RAW);
    return retFrame;
}

DVoid DX86_STDCALL DVideoFrame::Release(DVideoFrameData* pData)
{
    if (pData != _nullVideoFrameData)
    {
        assert(pData->nRefs != 0);
        DAtomDec32(&pData->nRefs);
        if (pData->nRefs <= 0)
        {
            free((void*)pData);
        }
    }
}

const DVideoFrame& DX86_STDCALL DVideoFrame::GetNullVideoFrame()
{
    return *(DVideoFrame*)&_nullVideoFrame;
}

DVideoFrameData* DVideoFrame::GetData() const
{
    assert(m_pBuf != nullptr);
    return ((DVideoFrameData*)m_pBuf) - 1;
}

DBool DVideoFrame::AllocFrame(DInt32 w, DInt32 h, DPixelFmt fmt)
{
    assert(w >= 0 && h >= 0);

    if (GetData()->nRefs >= 1)
    {
        Release();
    }

    if (w == 0 || h == 0)
    {
        Init();
    }
    else
    {
        DInt32 lineSize = DefaultStride(w, fmt);
        DInt32 videoSize = lineSize * h;
        DVideoFrameData* pData = (DVideoFrameData*)malloc(sizeof(DVideoFrameData) + videoSize);
        if (pData == nullptr)
            return false;

        pData->nRefs = 1;
        pData->nAllocLength = videoSize;
        pData->m_width = w;
        pData->m_height = h;
        pData->m_fmt = fmt;
        pData->m_type = DMemType::RAW;
        pData->m_stride = lineSize;
        pData->m_cts = 0;
        pData->m_dts = 0;
        pData->m_pts = 0;
        m_pBuf = pData->buf();
    }

    return true;
}

DBool DVideoFrame::AllocFrame(DUInt32 data_size, DInt32 w, DInt32 h, DInt32 lineSize, DPixelFmt fmt, DMemType mtype)
{
    assert(w >= 0 && h >= 0);

    if (GetData()->nRefs >= 1)
    {
        Release();
    }

    if (w == 0 || h == 0)
    {
        Init();
    }
    else
    {

        DVideoFrameData* pData = (DVideoFrameData*)malloc(sizeof(DVideoFrameData) + data_size);
        if (pData == nullptr)
            return false;

        pData->nRefs = 1;
        pData->nAllocLength = data_size;
        pData->m_width = w;
        pData->m_height = h;
        pData->m_fmt = fmt;
        pData->m_type = mtype;
        pData->m_stride = lineSize;
        pData->m_cts = 0;
        pData->m_dts = 0;
        pData->m_pts = 0;
        m_pBuf = pData->buf();
    }

    return true;
}

DBool DVideoFrame::AllocFrame(const DVideoFrame& frame_ref)
{
    if (GetData()->nRefs >= 1)
    {
        Release();
    }

    if (frame_ref.IsNull())
    {
        Init();
    }
    else
    {
        DVideoFrameData* pData = (DVideoFrameData*)malloc(sizeof(DVideoFrameData) + frame_ref.GetSize());
        if (pData == nullptr)
            return false;

        DXP::memcpy(pData, frame_ref.GetData(), sizeof(DVideoFrameData));
        pData->nRefs = 1;
        m_pBuf = pData->buf();
    }

    return true;
}

DBool DVideoFrame::AllocBeforeWrite(const DVideoFrame& bufSrc)
{
    DBool bRet = true;
    if (GetData()->nRefs > 1)
    {
        Release();
        bRet = AllocFrame(bufSrc);
    }
    assert(GetData()->nRefs <= 1);
    return bRet;
}

DVoid DVideoFrame::CopyBeforeWrite()
{
    if (GetData()->nRefs > 1)
    {
        DVideoFrameData* pData = GetData();
        Release();
        if (AllocFrame(*this))
        {
            DXP::memcpy(m_pBuf, pData->buf(), pData->nAllocLength);
        }
    }
    assert(GetData()->nRefs <= 1);
}
