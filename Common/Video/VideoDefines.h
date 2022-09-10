#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "Video/WinDS.h"

enum class DPixelFmt {
    Unknown,
    RGB24,
    ARGB,
    BGRA,
    RGB565,
    I420,
    IYUV,
    YUY2, // [Y0,U0,Y1,V0]
    MJPG
};

struct tagDVideoFormat {
    DInt32 width;
    DInt32 height;
    DInt32 max_fps;
    DPixelFmt format;
    DBool interlaced;
    DUInt32 dshow_fmt_idx;
    DBool support_fps_control;
    DBool videoinfo2;
    tagDVideoFormat() {
        width = 0;
        height = 0;
        max_fps = 0;
        format = DPixelFmt::Unknown;
        interlaced = false;
        dshow_fmt_idx = 0;
        support_fps_control = false;
        videoinfo2 = false;
    }
    bool operator==(const tagDVideoFormat& other) const {
        if (width == other.width && height == other.height && max_fps == other.max_fps
            && format == other.format && interlaced == other.interlaced)
            return true;
        return false;
    }
    bool operator!=(const tagDVideoFormat& other) const {
        return !operator==(other);
    }
};

typedef struct tagDVideoFormat DVideoFormat;

/*
class DVideoFrame 
{
public:
    DVideoFrame(DByte* data, DUInt32 data_size, DInt32 w, DInt32 h, DPixelFmt fmt) : m_data(data, data_size) {
        m_width = w;
        m_height = h;
        m_fmt = fmt;
        m_stride = DefaultStride(m_width, m_fmt);
    }
   
    DVideoFrame(const DVideoFrame& frame) {
        m_width = frame.m_width;
        m_height = frame.m_height;
        m_fmt = frame.m_fmt;
        m_stride = frame.m_stride;
        m_data = frame.m_data;
    }

    static DInt32 DefaultStride(DInt32 width, DPixelFmt fmt) {
        switch (fmt)
        {
        case DPixelFmt::I420:
        case DPixelFmt::IYUV:
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
    DInt32 m_width;
    DInt32 m_height;
    DPixelFmt m_fmt;
    DInt32 m_stride;
    DBuffer m_data;
};*/