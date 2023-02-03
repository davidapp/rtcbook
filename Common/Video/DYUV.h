﻿#pragma once

#include "DTypes.h"
#include "DVideoDefines.h"

class DRGB2YUV {
public: // for BT.601 Limited Range
    static DInt32 RGBToY(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 RGBToU(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 RGBToV(DUInt8 r, DUInt8 g, DUInt8 b);
    // BGR24
    static DVoid RAWToYRow(const DUInt8* src_argb0, DUInt8* dst_y, DInt32 width);
    static DVoid RAWToUVRow(const DUInt8* src_rgb0, DInt32 src_stride_rgb, DUInt8* dst_u, DUInt8* dst_v, DInt32 width);
    // TODO RGB24 ARGB BGRA ABGR RGBA

    // TODO RGBToYJ
};

class DYUV2RGB {
public:
    static DVoid YUV2RAW_BT601(DUInt8* out, DInt32 Y, DInt32 U, DInt32 V);
    static DVoid YUV2RAW_JPEG(DUInt8* out, DInt32 Y, DInt32 U, DInt32 V);
    static DVoid YUV2RAW_BT709(DUInt8* out, DInt32 Y, DInt32 U, DInt32 V);
    static DVoid YUV2RAW_QUICK(DUInt8* out, DInt32 Y, DInt32 U, DInt32 V);
};


class DYUV {
public:
    static DInt32 YUY2ToI420(const DByte* src_yuy2, DInt32 src_stride_yuy2,
        DByte* dst_y, DInt32 dst_stride_y, DByte* dst_u, DInt32 dst_stride_u, DByte* dst_v, DInt32 dst_stride_v,
        DInt32 width, DInt32 height);

public:
    static DInt32  Clamp0(DInt32 v);
    static DInt32  Clamp255(DInt32 v);
    static DInt32  Clamp1023(DInt32 v);

    static DUInt32 Clamp(DInt32 v);
    static DUInt32 Clamp10(DInt32 v);
};
