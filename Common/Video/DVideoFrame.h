#pragma once

#include "DTypes.h"
#include "DVideoDefines.h"


struct DVideoFrameData
{
    DAtomInt32 nRefs;
    DInt32 nAllocLength;
    DInt32 m_width;
    DInt32 m_height;
    DInt32 m_lineSize;
    DPixelFmt m_fmt;
    DMemType m_type;
    DUInt64 m_cts;
    DUInt64 m_dts;
    DUInt64 m_pts;
    DRotation m_rotate;
    DByte* m_pSEIData;
    DUInt32 m_nSEISize;
    DVoid* m_pUserData;

    DByte* buf()
    {
        return (DByte*)(this + 1);
    }
};

class DVideoFrame
{
public:
    DVideoFrame();
    explicit DVideoFrame(DInt32 w, DInt32 h, DPixelFmt fmt);
    DVideoFrame(DByte* data, DUInt32 data_size, DInt32 w, DInt32 h, 
                       DPixelFmt fmt, DMemType mtype = DMemType::RAW);
    ~DVideoFrame();
    DVoid Init();

public:
    DVideoFrame(const DVideoFrame& frame);
    DVideoFrame& operator =(const DVideoFrame& bufSrc);

public:
    DInt32 GetWidth() const;
    DInt32 GetHeight() const;
    DPixelFmt GetFormat() const;
    DMemType GetMemType() const;
    DInt32 GetLineSize() const;
    DUInt64 GetCTS() const;
    DVoid SetCTS(DUInt64 ts);
    DUInt64 GetDTS() const;
    DVoid SetDTS(DUInt64 ts);
    DUInt64 GetPTS() const;
    DVoid SetPTS(DUInt64 ts);
    DVoid* GetUserData() const;
    DVoid SetUserData(DVoid* pData);

public:
    DUInt32 GetRefCount() const;
    DByte* GetBuf() const;
    DUInt32 GetSize() const;
    DVoid AddRef();
    DVoid Release();
    DVoid Attach(DByte* pBuf);
    DVoid Detach();
    DBool IsNull() const;

public:
    DVoid  SetAt(DUInt32 index, DByte v);
    DByte  GetAt(DUInt32 index);
    DVoid  Zero();

public:
    DVideoFrame Copy();
    DBITMAPINFOHEADER* NewBMPInfoHeader();

public:
    static DVoid DX86_STDCALL Release(DVideoFrameData* pData);
    static const DVideoFrame& DX86_STDCALL GetNullVideoFrame();
    static DInt32 DefaultLineSize(DInt32 width, DPixelFmt fmt);

protected:
    DByte* m_pBuf;
    DVideoFrameData* GetData() const;

protected:
    DBool AllocFrame(DInt32 w, DInt32 h, DPixelFmt fmt);
    DBool AllocFrame(DUInt32 data_size, DInt32 w, DInt32 h, DInt32 lineSize, DPixelFmt fmt, DMemType mtype);
    DBool AllocFrame(const DVideoFrame& frame_ref);

    DBool AllocBeforeWrite(const DVideoFrame& bufSrc);
    DVoid CopyBeforeWrite();
};


typedef DVoid* (*VideoCallback)(DVideoFrame frame, DVoid* pUserData);
