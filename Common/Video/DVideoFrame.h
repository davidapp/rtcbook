#pragma once

#include "DTypes.h"
#include "VideoDefines.h"

typedef DVoid* (*VideoCallback)(DVoid* pFrame, DVoid* pFrameData, DVoid* pUserData);

class DVideoFrame
{
public:
    DVideoFrame(DByte* data, DUInt32 data_size, DInt32 w, DInt32 h, DPixelFmt fmt);
    DVideoFrame(const DVideoFrame& frame);
    DInt32 m_width;
    DInt32 m_height;
    DPixelFmt m_fmt;
    DInt32 m_stride;
    DBuffer m_data;
    DUInt64 m_cts;
    DUInt64 m_dts;
    DUInt64 m_pts;
public:
    static DVideoFrame* YUY2ToRAW(const DVideoFrame* buf);
    static DInt32 DefaultStride(DInt32 width, DPixelFmt fmt);
};
