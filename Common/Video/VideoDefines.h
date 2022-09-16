#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"
#include "Video/WinDS.h"
#include "File/DBmpFile.h"

#define WM_ONFRAME WM_USER+100

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
    DBITMAPINFOHEADER bmp_header;
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
