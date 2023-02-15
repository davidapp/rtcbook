#include "DVideoColor.h"

#define AVGB(a, b) (((a) + (b) + 1) >> 1)


DInt32 DRGB2YUV::RGBToY(DUInt8 r, DUInt8 g, DUInt8 b)
{
    return (66 * r + 129 * g + 25 * b + 0x1080) >> 8;
}

DInt32 DRGB2YUV::RGBToU(DUInt8 r, DUInt8 g, DUInt8 b)
{
    return (112 * b - 74 * g - 38 * r + 0x8000) >> 8;
}

DInt32 DRGB2YUV::RGBToV(DUInt8 r, DUInt8 g, DUInt8 b)
{
    return (112 * r - 94 * g - 18 * b + 0x8000) >> 8;
}

DVoid DRGB2YUV::RAWToYRow(const DUInt8* src_argb0, DUInt8* dst_y, DInt32 width)
{
    for (DInt32 x = 0; x < width; ++x)
    {
        dst_y[0] = DRGB2YUV::RGBToY(src_argb0[2], src_argb0[1], src_argb0[0]);
        src_argb0 += 3;
        dst_y += 1;
    }
}

DVoid DRGB2YUV::RAWToUVRow(const DUInt8* src_rgb0, DInt32 src_stride_rgb, DUInt8* dst_u, DUInt8* dst_v, DInt32 width)
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

// Jpeg uses a variation on BT.601-1 full range
// y =  0.29900 * r + 0.58700 * g + 0.11400 * b
// u = -0.16874 * r - 0.33126 * g + 0.50000 * b  + center
// v =  0.50000 * r - 0.41869 * g - 0.08131 * b  + center

DInt32 DRGB2YUV::DRGB2YUV::RGBToYJ(DUInt8 r, DUInt8 g, DUInt8 b) {
    return (38 * r + 75 * g + 15 * b + 64) >> 7;
}

DInt32 DRGB2YUV::DRGB2YUV::RGBToUJ(DUInt8 r, DUInt8 g, DUInt8 b) {
    return (127 * b - 84 * g - 43 * r + 0x8080) >> 8;
}

DInt32 DRGB2YUV::RGBToVJ(DUInt8 r, DUInt8 g, DUInt8 b) {
    return (127 * r - 107 * g - 20 * b + 0x8080) >> 8;
}

//  BT.601 YUV to RGB reference
//  R = (Y - 16) * 1.164              - V * -1.596
//  G = (Y - 16) * 1.164 - U *  0.391 - V *  0.813
//  B = (Y - 16) * 1.164 - U * -2.018

DVoid DYUV2RGB::BT601(DUInt8* out, DInt32 y, DInt32 u, DInt32 v)
{
    DUInt32 y1 = (DUInt32)(y * 0x0101 * 18997) >> 16;
    *out = DYUV::Clamp((DInt32)(-(u * -128) + y1 + -17544) >> 6);
    *(out+1) = DYUV::Clamp((DInt32)(-(u * 25 + v * 52) + y1 + 8696) >> 6);
    *(out+2) = DYUV::Clamp((DInt32)(-(v * -102) + y1 + -14216) >> 6);
}

// JPEG YUV to RGB reference
// R = Y                - V * -1.40200
// G = Y - U *  0.34414 - V *  0.71414
// B = Y - U * -1.77200

DVoid DYUV2RGB::JPEG(DUInt8* out, DInt32 y, DInt32 u, DInt32 v)
{
    uint32_t y1 = (uint32_t)(y * 0x0101 * 16320) >> 16;
    *out = DYUV::Clamp((int32_t)(-(u * -113) + y1 + -14432) >> 6);
    *(out + 1) = DYUV::Clamp((int32_t)(-(u * 22 + v * 46) + y1 + 8736) >> 6);
    *(out + 2) = DYUV::Clamp((int32_t)(-(v * -90) + y1 + -11488) >> 6);
}

// BT.709 YUV to RGB reference
// R = (Y - 16) * 1.164              - V * -1.793
// G = (Y - 16) * 1.164 - U *  0.213 - V *  0.533
// B = (Y - 16) * 1.164 - U * -2.112

DVoid DYUV2RGB::BT709(DUInt8* out, DInt32 y, DInt32 u, DInt32 v)
{
    uint32_t y1 = (uint32_t)(y * 0x0101 * 18997) >> 16;
    *out = DYUV::Clamp((int32_t)(-(u * -128) + y1 + -17544) >> 6);
    *(out+1) = DYUV::Clamp((int32_t)(-(u * 14 + v * 34) + y1 + 4984) >> 6);
    *(out+2) = DYUV::Clamp((int32_t)(-(v * -115) + y1 + -15880) >> 6);
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
