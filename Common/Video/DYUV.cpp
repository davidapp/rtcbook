﻿#include "DYUV.h"


DInt32 DYUV::RGBToY(uint8_t r, uint8_t g, uint8_t b) 
{
    return (66 * r + 129 * g + 25 * b + 0x1080) >> 8;
}

DInt32 DYUV::RGBToU(DUInt8 r, DUInt8 g, DUInt8 b)
{
    return (112 * b - 74 * g - 38 * r + 0x8000) >> 8;
}

DInt32 DYUV::RGBToV(DUInt8 r, DUInt8 g, DUInt8 b)
{
    return (112 * r - 94 * g - 18 * b + 0x8000) >> 8;
}

DVoid DYUV::BGR24ToYRow(const DUInt8* src_argb0, DUInt8* dst_y, DInt32 width)
{
    for (DInt32 x = 0; x < width; ++x)
    {
        dst_y[0] = DYUV::RGBToY(src_argb0[2], src_argb0[1], src_argb0[0]);
        src_argb0 += 3;
        dst_y += 1;
    }
}

#define AVGB(a, b) (((a) + (b) + 1) >> 1)

DVoid DYUV::BGR24ToUVRow(const DUInt8* src_rgb0, DInt32 src_stride_rgb, DUInt8* dst_u, DUInt8* dst_v, DInt32 width)
{
    const DUInt8* src_rgb1 = src_rgb0 + src_stride_rgb;
    for (DInt32 x = 0; x < width - 1; x += 2)
    {
        DUInt8 ab = AVGB(AVGB(src_rgb0[0], src_rgb1[0]), AVGB(src_rgb0[3], src_rgb1[3]));
        DUInt8 ag = AVGB(AVGB(src_rgb0[1], src_rgb1[1]), AVGB(src_rgb0[4], src_rgb1[4]));
        DUInt8 ar = AVGB(AVGB(src_rgb0[2], src_rgb1[2]), AVGB(src_rgb0[5], src_rgb1[5]));

        dst_u[0] = RGBToU(ar, ag, ab);
        dst_v[0] = RGBToV(ar, ag, ab);
        src_rgb0 += 3 * 2;
        src_rgb1 += 3 * 2;
        dst_u += 1;
        dst_v += 1;
    }

    if (width & 1)
    {
        DUInt8 ab = AVGB(src_rgb0[0], src_rgb1[0]);
        DUInt8 ag = AVGB(src_rgb0[1], src_rgb1[1]);
        DUInt8 ar = AVGB(src_rgb0[2], src_rgb1[2]);
        dst_u[0] = RGBToU(ar, ag, ab);
        dst_v[0] = RGBToV(ar, ag, ab);
    }
}

// JPeg uses a variation on BT.601-1 full range
// y =  0.29900 * r + 0.58700 * g + 0.11400 * b
// u = -0.16874 * r - 0.33126 * g + 0.50000 * b  + center
// v =  0.50000 * r - 0.41869 * g - 0.08131 * b  + center

DInt32 DYUV::RGBToYJ(DUInt8 r, DUInt8 g, DUInt8 b)
{
    return (77 * r + 150 * g + 29 * b + 128) >> 8;
}

DInt32 DYUV::RGBToUJ(DUInt8 r, DUInt8 g, DUInt8 b)
{
    return (127 * b - 84 * g - 43 * r + 0x8080) >> 8;
}

DInt32 DYUV::RGBToVJ(DUInt8 r, DUInt8 g, DUInt8 b)
{
    return (127 * r - 107 * g - 20 * b + 0x8080) >> 8;
}

DVoid DYUV::BGR24ToYJRow(const DUInt8* src_argb0, DUInt8* dst_y, DInt32 width)
{
    for (DInt32 x = 0; x < width; ++x)
    {
        dst_y[0] = DYUV::RGBToYJ(src_argb0[2], src_argb0[1], src_argb0[0]);
        src_argb0 += 3;
        dst_y += 1;
    }
}

DVoid DYUV::BGR24ToUVJRow(const DUInt8* src_rgb0, DInt32 src_stride_rgb, DUInt8* dst_u, DUInt8* dst_v, DInt32 width)
{
    const DUInt8* src_rgb1 = src_rgb0 + src_stride_rgb;
    for (DInt32 x = 0; x < width - 1; x += 2) {
        DUInt8 ab = AVGB(AVGB(src_rgb0[0], src_rgb1[0]), AVGB(src_rgb0[0 + 3], src_rgb1[0 + 3]));
        DUInt8 ag = AVGB(AVGB(src_rgb0[1], src_rgb1[1]), AVGB(src_rgb0[1 + 3], src_rgb1[1 + 3]));
        DUInt8 ar = AVGB(AVGB(src_rgb0[2], src_rgb1[2]), AVGB(src_rgb0[2 + 3], src_rgb1[2 + 3]));
        dst_u[0] = RGBToUJ(ar, ag, ab);
        dst_v[0] = RGBToVJ(ar, ag, ab);
        src_rgb0 += 3 * 2;
        src_rgb1 += 3 * 2;
        dst_u += 1;
        dst_v += 1;
    }
    if (width & 1) 
    {
        DUInt8 ab = AVGB(src_rgb0[0], src_rgb1[0]);
        DUInt8 ag = AVGB(src_rgb0[1], src_rgb1[1]);
        DUInt8 ar = AVGB(src_rgb0[2], src_rgb1[2]);
        dst_u[0] = RGBToUJ(ar, ag, ab);
        dst_v[0] = RGBToVJ(ar, ag, ab);
    }
}


DVoid DYUV::ARGBGrayRow(const DUInt8* src_argb, DUInt8* dst_argb, DInt32 width)
{
    for (DInt32 x = 0; x < width; ++x) 
    {
        uint8_t y = DYUV::RGBToYJ(src_argb[2], src_argb[1], src_argb[0]);
        dst_argb[2] = dst_argb[1] = dst_argb[0] = y;
        dst_argb[3] = src_argb[3];
        dst_argb += 4;
        src_argb += 4;
    }
}


DVoid DYUV::YUV2RGB(DUInt8* out, DInt32 Y, DInt32 U, DInt32 V)
{
    out[2] = Clamp(Y + (91881 * V + 32768 >> 16));
    out[1] = Clamp(Y + (- 22554 * U - 46802 * V + 32768 >> 16));
    out[0] = Clamp(Y + (116130 * U + 32768 >> 16));
}

DInt32 DYUV::Clamp0(DInt32 v)
{
    // return (v < 0) ? 0 : v;
    return ((-(v) >> 31) & (v));
}

DInt32 DYUV::Clamp255(DInt32 v)
{
    // return (v > 255) ? 255 : v;
    return (((255 - (v)) >> 31) | (v)) & 255;
}

DInt32 DYUV::Clamp1023(DInt32 v)
{
    //return (v > 1023) ? 1023 : v;
    return (((1023 - (v)) >> 31) | (v)) & 1023;
}

DUInt32 DYUV::Clamp(DInt32 v)
{
    // make 0<=v<=255
    DInt32 val = Clamp0(v);
    return (DUInt32)(Clamp255(val));
}

DUInt32 DYUV::Clamp10(DInt32 v)
{
    // make 0<=v<=1023
    DInt32 val = Clamp0(v);
    return (DUInt32)(Clamp1023(val));
}
