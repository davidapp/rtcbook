#pragma once

#include "DTypes.h"

enum class DPixelFmt {
    Unknown,
    RGB24,
    ARGB,
    BGRA,
    RGB565,
    I420,
    IYUV,
    YUY2,
    MJPG
};

struct DVideoFormat {
    DInt32 width;
    DInt32 height;
    DInt32 max_fps;
    DPixelFmt format;
    DBool interlaced;
    DUInt32 dshow_fmt_idx;
    DBool support_fps_control;
    DBool videoinfo2;
    DVideoFormat() {
        width = 0;
        height = 0;
        max_fps = 0;
        format = DPixelFmt::Unknown;
        interlaced = false;
        dshow_fmt_idx = 0;
        support_fps_control = false;
        videoinfo2 = false;
    }
    bool operator==(const DVideoFormat& other) const {
        if (width == other.width && height == other.height && max_fps == other.max_fps
            && format == other.format && interlaced == other.interlaced)
            return true;
        return false;
    }
    bool operator!=(const DVideoFormat& other) const {
        return !operator==(other);
    }
};