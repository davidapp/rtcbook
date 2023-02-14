#pragma once

#include "DTypes.h"
#include "File/DBmpFile.h"

#define WM_ONFRAME WM_USER+100

enum class DPixelFmt {
    Unknown,
    RGB24,// [(B,G,R),(B,G,R)...]
    RAW,  // [(B,G,R),(B,G,R)...] 第一行是最后一行，每行4字节对齐
    ARGB, // [(B,G,R,A),...]
    ABGR, // [(R,G,B,A),...]
    BGRA, // [(A,R,G,B),...]
    RGBA, // [(A,B,G,R),...]
    RGB565, // 16 bit
    I420, // [Y,U,V] or called IYUV
    YV12, // [Y,V,U] 
    NV12, // [Y,U0,V0,U1,V1...]
    NV21, // [Y,V0,U0,V1,U1...]
    YUY2, // [Y0,U0,Y1,V0...] YUV422
    MJPG  // 压缩格式
};

enum class DMemType {
    RAW,
    CVPixelBuffer
};

enum class DRotation {    // 图像回正需要顺时针旋转的角度
    DEGREE_0 = 0,
    DEGREE_90 = 90,
    DEGREE_180 = 180,
    DEGREE_270 = 270
};

enum class DScaleMode {
    CROP = 0,
    FILL = 1,
    STRETCH = 2
};

enum class DColorSpace{
    BT601LimitedRange = 0,
    BT601FullRange,
    BT709LimitedRange,
    BT709FullRange
};

struct tagDImageFormat {
    DInt32 width;
    DInt32 height;
    DPixelFmt format;
    DBITMAPINFOHEADER bmp_header;

    tagDImageFormat() : bmp_header{0} {
        width = 0;
        height = 0;
        format = DPixelFmt::Unknown;
    }

    bool operator==(const tagDImageFormat& other) const {
        if (width == other.width && height == other.height && format == other.format)
            return true;
        return false;
    }

    bool operator!=(const tagDImageFormat& other) const {
        return !operator==(other);
    }
};

typedef struct tagDImageFormat DImageFormat;


class DRect
{
public:
    DRect() { left = 0; top = 0; right = 0; bottom = 0; };
    DRect(DInt32 l, DInt32 t, DInt32 r, DInt32 b) {
        left = l;
        top = t;
        right = r;
        bottom = b;
    }

    DInt32 Width() const { return right - left; };
    DInt32 Height() const { return bottom - top; };
public:
    DInt32 left;
    DInt32 top;
    DInt32 right;
    DInt32 bottom;
};