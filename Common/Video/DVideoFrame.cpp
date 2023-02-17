#include "DVideoFrame.h"
#include "Base/DXP.h"
#include <sstream>

////////////////////////////////////////////////////////////////////////////////////
// DVideoFrame

struct DVideoFrameMemory{
    DVideoFrameData data;
    DVoid* buf;
};
const DVideoFrameMemory nNullVideoFrame = { {-1, 0, 0, 0, 0, DPixelFmt::Unknown, DMemType::RAW,0,0,0,DRotation::DEGREE_0}, nullptr };
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
    AllocFrame(data_size, w, h, DVideoFrame::DefaultLineSize(w,fmt), fmt, mtype);
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
    if (frame.GetData()->nRefs > 0)
    {
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
    return GetData()->m_lineSize;
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

DVoid* DVideoFrame::GetUserData() const
{
    return GetData()->m_pUserData;
}

DVoid DVideoFrame::SetUserData(DVoid* pData)
{
    GetData()->m_pUserData = pData;
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
    m_pBuf[index] = v;
}

DByte DVideoFrame::GetAt(DUInt32 index)
{
    return m_pBuf[index];
}

DVoid DVideoFrame::Zero()
{
    CopyBeforeWrite();
    memset(m_pBuf, 0, GetSize());
}

DInt32 DVideoFrame::GetI420UV_Width()
{
    if (GetData()->m_fmt != DPixelFmt::I420) return 0;
    DInt32 w = GetData()->m_width;
    return (w % 2 == 0) ? w / 2 : w / 2 + 1;
}

DInt32 DVideoFrame::GetI420UV_Height()
{
    if (GetData()->m_fmt != DPixelFmt::I420) return 0;
    DInt32 h = GetData()->m_height;
    return (h % 2 == 0) ? h / 2 : h / 2 + 1;
}


DVideoFrame DVideoFrame::Copy()
{
    DVideoFrame frameRet(GetBuf(), GetSize(), GetData()->m_width, GetData()->m_height, GetData()->m_fmt, GetData()->m_type);
    return frameRet;
}

std::string DVideoFrame::GetDumpText()
{
    std::string strRet,strTemp;
    std::stringstream ss,sstemp;

    strRet += "nRefs: ";
    sstemp << GetData()->nRefs;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "nAllocLength: ";
    sstemp << GetData()->nAllocLength;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_width: ";
    sstemp << GetData()->m_width;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_height: ";
    sstemp << GetData()->m_height;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_lineSize: ";
    sstemp << GetData()->m_lineSize;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_fmt: ";
    sstemp << (int)GetData()->m_fmt;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    if (GetData()->m_fmt == DPixelFmt::Unknown) strRet += "(Unknown)";
    else if (GetData()->m_fmt == DPixelFmt::RGB24) strRet += "(RGB24)";
    else if (GetData()->m_fmt == DPixelFmt::RAW) strRet += "(RAW)";
    else if (GetData()->m_fmt == DPixelFmt::ARGB) strRet += "(ARGB)";
    else if (GetData()->m_fmt == DPixelFmt::ABGR) strRet += "(ABGR)";
    else if (GetData()->m_fmt == DPixelFmt::BGRA) strRet += "(BGRA)";
    else if (GetData()->m_fmt == DPixelFmt::RGBA) strRet += "(RGBA)";
    else if (GetData()->m_fmt == DPixelFmt::RGB565) strRet += "(RGB565)";
    else if (GetData()->m_fmt == DPixelFmt::I420) strRet += "(I420)";
    else if (GetData()->m_fmt == DPixelFmt::YV12) strRet += "(YV12)";
    else if (GetData()->m_fmt == DPixelFmt::NV12) strRet += "(NV12)";
    else if (GetData()->m_fmt == DPixelFmt::NV21) strRet += "(NV21)";
    else if (GetData()->m_fmt == DPixelFmt::YUY2) strRet += "(YUY2)";
    else if (GetData()->m_fmt == DPixelFmt::MJPG) strRet += "(MJPG)";
    strRet += D_LINES;

    strRet += "m_type: ";
    sstemp << (int)GetData()->m_type;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    if (GetData()->m_type == DMemType::RAW) strRet += "(RAW)";
    else if (GetData()->m_type == DMemType::CVPixelBuffer) strRet += "(CVPixelBuffer)";
    strRet += D_LINES;

    strRet += "m_cts: ";
    sstemp << GetData()->m_cts;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_dts: ";
    sstemp << GetData()->m_dts;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_pts: ";
    sstemp << GetData()->m_pts;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_rotate: ";
    sstemp << (int)GetData()->m_rotate;
    sstemp >> strTemp;
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_pSEIData: 0x";
    sstemp << std::hex << (DInt64)(GetData()->m_pSEIData);
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_nSEISize: ";
    sstemp << GetData()->m_nSEISize;
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;

    strRet += "m_pUserData: 0x";
    sstemp << std::hex << (DInt64)(GetData()->m_pUserData);
    sstemp >> strTemp;
    sstemp.clear();
    strRet += strTemp;
    strRet += D_LINES;



    return strRet;
}

DBuffer DVideoFrame::GetDumpBuffer()
{
    DBuffer bufRet;
    return bufRet;
}

DBool DVideoFrame::LoadFromBuffer(const DBuffer& buf)
{
    return true;
}

DBITMAPINFOHEADER* DVideoFrame::NewBMPInfoHeader()
{
    DBITMAPINFOHEADER* pHeader = new DBITMAPINFOHEADER();
    pHeader->biBitCount = 24;
    pHeader->biClrImportant = 0;
    pHeader->biClrUsed = 0;
    pHeader->biCompression = BI_RGB;
    pHeader->biHeight = GetData()->m_height;
    pHeader->biWidth = GetData()->m_width;
    pHeader->biPlanes = 3;
    pHeader->biSize = 40;
    pHeader->biSizeImage = GetData()->m_lineSize * GetData()->m_height;
    pHeader->biXPelsPerMeter = 0;
    pHeader->biYPelsPerMeter = 0;
    return pHeader;
}

DVoid DX86_STDCALL DVideoFrame::Release(DVideoFrameData* pData)
{
    if (pData != _nullVideoFrameData)
    {
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

DInt32 DVideoFrame::DefaultLineSize(DInt32 width, DPixelFmt fmt)
{
    DInt32 line = width * 3;
    switch (fmt)
    {
    case DPixelFmt::I420:
        return width;
    case DPixelFmt::RGB565:
    case DPixelFmt::YUY2:
        return width * 2;
    case DPixelFmt::RGB24:
        return line;
    case DPixelFmt::ARGB:
    case DPixelFmt::ABGR:
    case DPixelFmt::BGRA:
    case DPixelFmt::RGBA:
        return width * 4;
    case DPixelFmt::RAW:
        if (line % 4 != 0) {
            line += (4 - line % 4);
        }
        return line;
    default:
        return 0;
    }
}

DVideoFrameData* DVideoFrame::GetData() const
{
    return ((DVideoFrameData*)m_pBuf) - 1;
}

DBool DVideoFrame::AllocFrame(DInt32 w, DInt32 h, DPixelFmt fmt)
{
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
        DInt32 lineSize = DVideoFrame::DefaultLineSize(w, fmt);
        DInt32 videoSize = lineSize * h;
        if (fmt == DPixelFmt::I420) {
            DInt32 ySize = w * h;
            DInt32 uLine = (w % 2 == 0) ? w / 2 : w / 2 + 1;
            DInt32 uHeight = (h % 2 == 0) ? h / 2 : h / 2 + 1;
            videoSize = ySize + uLine * uHeight * 2;
        }
        DVideoFrameData* pData = (DVideoFrameData*)malloc(sizeof(DVideoFrameData) + videoSize);
        if (pData == nullptr)
            return false;

        pData->nRefs = 1;
        pData->nAllocLength = videoSize;
        pData->m_width = w;
        pData->m_height = h;
        pData->m_fmt = fmt;
        pData->m_type = DMemType::RAW;
        pData->m_lineSize = lineSize;
        pData->m_cts = 0;
        pData->m_dts = 0;
        pData->m_pts = 0;
        pData->m_rotate = DRotation::DEGREE_0;
        pData->m_pSEIData = nullptr;
        pData->m_nSEISize = 0;
        pData->m_pUserData = nullptr;
        m_pBuf = pData->buf();
    }

    return true;
}

DBool DVideoFrame::AllocFrame(DUInt32 data_size, DInt32 w, DInt32 h, DInt32 lineSize, DPixelFmt fmt, DMemType mtype)
{
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
        pData->m_lineSize = lineSize;
        pData->m_cts = 0;
        pData->m_dts = 0;
        pData->m_pts = 0;
        pData->m_rotate = DRotation::DEGREE_0;
        pData->m_pSEIData = nullptr;
        pData->m_nSEISize = 0;
        pData->m_pUserData = nullptr;
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
}
