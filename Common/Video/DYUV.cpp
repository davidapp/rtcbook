#include "DYUV.h"


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


// BT.601 YUV to RGB reference
//  R = (Y - 16) * 1.164              - V * -1.596
//  G = (Y - 16) * 1.164 - U *  0.391 - V *  0.813
//  B = (Y - 16) * 1.164 - U * -2.018

// Y contribution to R,G,B.  Scale and bias.
#define YG 18997  /* round(1.164 * 64 * 256 * 256 / 257) */
#define YGB -1160 /* 1.164 * 64 * -16 + 64 / 2 */

// U and V contributions to R,G,B.
#define UB -128 /* max(-128, round(-2.018 * 64)) */
#define UG 25   /* round(0.391 * 64) */
#define VG 52   /* round(0.813 * 64) */
#define VR -102 /* round(-1.596 * 64) */

// Bias values to subtract 16 from Y and 128 from U and V.
#define BB (UB * 128 + YGB)
#define BG (UG * 128 + VG * 128 + YGB)
#define BR (VR * 128 + YGB)

struct YuvConstants {
    int8_t kUVToB[32];
    int8_t kUVToG[32];
    int8_t kUVToR[32];
    int16_t kUVBiasB[16];
    int16_t kUVBiasG[16];
    int16_t kUVBiasR[16];
    int16_t kYToRgb[16];
    int16_t kYBiasToRgb[16];
};

const struct YuvConstants kYuvI601Constants = {
    {UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0,
     UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0, UB, 0},
    {UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG,
     UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG, UG, VG},
    {0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR,
     0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR, 0, VR},
    {BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB, BB},
    {BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG, BG},
    {BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR, BR},
    {YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG, YG},
    {YGB, YGB, YGB, YGB, YGB, YGB, YGB, YGB, YGB, YGB, YGB, YGB, YGB, YGB, YGB,
     YGB} };

DVoid DYUV::YuvPixel(DUInt8 y, DUInt8 u, DUInt8 v, DUInt8* b, DUInt8* g, DUInt8* r)
{
    int ub = kYuvI601Constants.kUVToB[0];
    int ug = kYuvI601Constants.kUVToG[0];
    int vg = kYuvI601Constants.kUVToG[1];
    int vr = kYuvI601Constants.kUVToR[1];
    int bb = kYuvI601Constants.kUVBiasB[0];
    int bg = kYuvI601Constants.kUVBiasG[0];
    int br = kYuvI601Constants.kUVBiasR[0];
    int yg = kYuvI601Constants.kYToRgb[0];

    DUInt32 y1 = (DUInt32)(y * 0x0101 * yg) >> 16;
    *b = Clamp((DInt32)(-(u * ub) + y1 + bb) >> 6);
    *g = Clamp((DInt32)(-(u * ug + v * vg) + y1 + bg) >> 6);
    *r = Clamp((DInt32)(-(v * vr) + y1 + br) >> 6);
}

DVoid DYUV::I420ToARGBRow(const DUInt8* src_y, const DUInt8* src_u, const DUInt8* src_v, DUInt8* rgb_buf, DInt32 width)
{
    for (DInt32 x = 0; x < width - 1; x += 2) 
    {
        YuvPixel(src_y[0], src_u[0], src_v[0], rgb_buf + 0, rgb_buf + 1, rgb_buf + 2);
        rgb_buf[3] = 255;
        YuvPixel(src_y[1], src_u[0], src_v[0], rgb_buf + 4, rgb_buf + 5, rgb_buf + 6);
        rgb_buf[7] = 255;
        src_y += 2;
        src_u += 1;
        src_v += 1;
        rgb_buf += 8;  // Advance 2 pixels.
    }
    if (width & 1) 
    {
        YuvPixel(src_y[0], src_u[0], src_v[0], rgb_buf + 0, rgb_buf + 1, rgb_buf + 2);
        rgb_buf[3] = 255;
    }
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
