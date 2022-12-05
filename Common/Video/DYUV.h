#pragma once

#include "DTypes.h"
#include "VideoDefines.h"

class DYUV
{
public:
    static DInt32 RGBToY(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 RGBToU(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 RGBToV(DUInt8 r, DUInt8 g, DUInt8 b);

    static DVoid BGR24ToYRow(const DUInt8* src_argb0, DUInt8* dst_y, DInt32 width);
    static DVoid BGR24ToUVRow(const DUInt8* src_rgb0, DInt32 src_stride_rgb, DUInt8* dst_u, DUInt8* dst_v, DInt32 width);

public:
    static DInt32 DYUV::RGBToYJ(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 DYUV::RGBToUJ(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 DYUV::RGBToVJ(DUInt8 r, DUInt8 g, DUInt8 b);

    static DVoid BGR24ToYJRow(const DUInt8* src_argb0, DUInt8* dst_y, DInt32 width);
    static DVoid BGR24ToUVJRow(const DUInt8* src_rgb0, DInt32 src_stride_rgb, DUInt8* dst_u, DUInt8* dst_v, DInt32 width);

public:
    static DVoid ARGBGrayRow(const DUInt8* src_argb, DUInt8* dst_argb, DInt32 width);

public:
    static DVoid YuvPixel(DUInt8 y, DUInt8 u, DUInt8 v, DUInt8* b, DUInt8* g, DUInt8* r);
    static DVoid I420ToARGBRow(const DUInt8* src_y, const DUInt8* src_u, const DUInt8* src_v, DUInt8* rgb_buf, DInt32 width);

public:
    static DInt32  Clamp0(DInt32 v);
    static DInt32  Clamp255(DInt32 v);
    static DInt32  Clamp1023(DInt32 v);

    static DUInt32 Clamp(DInt32 v);
    static DUInt32 Clamp10(DInt32 v);
};
