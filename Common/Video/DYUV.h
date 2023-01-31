#pragma once

#include "DTypes.h"
#include "DVideoDefines.h"

class DYUV
{
public:
    static DInt32 RGBToY(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 RGBToU(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 RGBToV(DUInt8 r, DUInt8 g, DUInt8 b);

    static DVoid BGR24ToYRow(const DUInt8* src_argb0, DUInt8* dst_y, DInt32 width);
    static DVoid BGR24ToUVRow(const DUInt8* src_rgb0, DInt32 src_stride_rgb, DUInt8* dst_u, DUInt8* dst_v, DInt32 width);

public:
    static DInt32 RGBToYJ(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 RGBToUJ(DUInt8 r, DUInt8 g, DUInt8 b);
    static DInt32 RGBToVJ(DUInt8 r, DUInt8 g, DUInt8 b);

    static DVoid BGR24ToYJRow(const DUInt8* src_argb0, DUInt8* dst_y, DInt32 width);
    static DVoid BGR24ToUVJRow(const DUInt8* src_rgb0, DInt32 src_stride_rgb, DUInt8* dst_u, DUInt8* dst_v, DInt32 width);

public:
    static DVoid ARGBGrayRow(const DUInt8* src_argb, DUInt8* dst_argb, DInt32 width);

public:
    static DVoid YUV2RGB(DUInt8* out, DInt32 Y, DInt32 U, DInt32 V);

public:
    static DInt32  Clamp0(DInt32 v);
    static DInt32  Clamp255(DInt32 v);
    static DInt32  Clamp1023(DInt32 v);

    static DUInt32 Clamp(DInt32 v);
    static DUInt32 Clamp10(DInt32 v);
};
