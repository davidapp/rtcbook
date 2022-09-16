#pragma once

#include "DTypes.h"
#include "VideoDefines.h"

class DVideoFrame
{
public:
    DVideoFrame(DByte* data, DUInt32 data_size, DInt32 w, DInt32 h, DPixelFmt fmt);
    DVideoFrame(const DVideoFrame& frame);
    static DInt32 DefaultStride(DInt32 width, DPixelFmt fmt);
    DInt32 m_width;
    DInt32 m_height;
    DPixelFmt m_fmt;
    DInt32 m_stride;
    DBuffer m_data;
};
