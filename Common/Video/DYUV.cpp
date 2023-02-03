#include "DYUV.h"

#define AVGB(a, b) (((a) + (b) + 1) >> 1)


DInt32 DRGB2YUV::RGBToY(uint8_t r, uint8_t g, uint8_t b)
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

//  BT.601 YUV to RGB reference
//  R = (Y - 16) * 1.164              - V * -1.596
//  G = (Y - 16) * 1.164 - U *  0.391 - V *  0.813
//  B = (Y - 16) * 1.164 - U * -2.018

DVoid DYUV2RGB::YUV2RAW_BT601(DUInt8* out, DInt32 y, DInt32 u, DInt32 v)
{
    DUInt32 y1 = (DUInt32)(y * 0x0101 * 18997) >> 16;
    *out = DYUV::Clamp((DInt32)(-(u * -128) + y1 + -17544) >> 6);
    *(out+1) = DYUV::Clamp((DInt32)(-(u * 25 + v * 52) + y1 + 8696) >> 6);
    *(out+2) = DYUV::Clamp((DInt32)(-(v * -102) + y1 + -14216) >> 6);
}

// JPEG YUV to RGB reference
// *  R = Y                - V * -1.40200
// *  G = Y - U *  0.34414 - V *  0.71414
// *  B = Y - U * -1.77200

DVoid DYUV2RGB::YUV2RAW_JPEG(DUInt8* out, DInt32 y, DInt32 u, DInt32 v)
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
// See http://www.equasys.de/colorconversion.html

DVoid DYUV2RGB::YUV2RAW_BT709(DUInt8* out, DInt32 y, DInt32 u, DInt32 v)
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

// Copy row of YUY2 Y's (422) into Y (420/422).
DVoid YUY2ToYRow(const DByte* src_yuy2, DByte* dst_y, DInt32 width)
{
    // Output a row of Y values.
    for (DInt32 x = 0; x < width - 1; x += 2) {
        dst_y[x] = src_yuy2[0];
        dst_y[x + 1] = src_yuy2[2];
        src_yuy2 += 4;
    }
    if (width & 1) {
        dst_y[width - 1] = src_yuy2[0];
    }
}

DVoid YUY2ToUVRow(const DByte* src_yuy2, DInt32 src_stride_yuy2,
    DByte* dst_u, DByte* dst_v, DInt32 width)
{
    // Output a row of UV values, filtering 2 rows of YUY2.
    for (DInt32 x = 0; x < width; x += 2) 
    {
        dst_u[0] = (src_yuy2[1] + src_yuy2[src_stride_yuy2 + 1] + 1) >> 1;
        dst_v[0] = (src_yuy2[3] + src_yuy2[src_stride_yuy2 + 3] + 1) >> 1;
        src_yuy2 += 4;
        dst_u += 1;
        dst_v += 1;
    }
}

DInt32 DYUV::YUY2ToI420(const DByte* src_yuy2, DInt32 src_stride_yuy2,
    DByte* dst_y, DInt32 dst_stride_y, DByte* dst_u, DInt32 dst_stride_u, DByte* dst_v, DInt32 dst_stride_v,
    DInt32 width, DInt32 height)
{
    // Negative height means invert the image.
    if (height < 0) {
        height = -height;
        src_yuy2 = src_yuy2 + (height - 1) * src_stride_yuy2;
        src_stride_yuy2 = -src_stride_yuy2;
    }

    for (DInt32 y = 0; y < height - 1; y += 2) {
        YUY2ToUVRow(src_yuy2, src_stride_yuy2, dst_u, dst_v, width);
        YUY2ToYRow(src_yuy2, dst_y, width);
        YUY2ToYRow(src_yuy2 + src_stride_yuy2, dst_y + dst_stride_y, width);
        src_yuy2 += src_stride_yuy2 * 2;
        dst_y += dst_stride_y * 2;
        dst_u += dst_stride_u;
        dst_v += dst_stride_v;
    }
    if (height & 1) {
        YUY2ToUVRow(src_yuy2, 0, dst_u, dst_v, width);
        YUY2ToYRow(src_yuy2, dst_y, width);
    }
    return 0;
}

DVoid I420ToARGBRow(const DByte* src_y, const DByte* src_u, const DByte* src_v,
    DByte* rgb_buf, DInt32 width) 
{
    for (DInt32 x = 0; x < width - 1; x += 2) {
        DYUV2RGB::YUV2RAW_BT601(rgb_buf, src_y[0], src_u[0], src_v[0]);
        rgb_buf[3] = 255;
        DYUV2RGB::YUV2RAW_BT601(rgb_buf + 4, src_y[1], src_u[0], src_v[0]);
        rgb_buf[7] = 255;
        src_y += 2;
        src_u += 1;
        src_v += 1;
        rgb_buf += 8;
    }
    if (width & 1) {
        DYUV2RGB::YUV2RAW_BT601(rgb_buf, src_y[0], src_u[0], src_v[0]);
        rgb_buf[3] = 255;
    }
}

DInt32 I420ToARGB(const DByte* src_y, DInt32 src_stride_y,
    const DByte* src_u, DInt32 src_stride_u,
    const DByte* src_v, DInt32 src_stride_v,
    DByte* dst_argb, DInt32 dst_stride_argb,
    DInt32 width, DInt32 height) 
{
    if (!src_y || !src_u || !src_v || !dst_argb || width <= 0 || height == 0) {
        return -1;
    }
    // Negative height means invert the image.
    if (height < 0) 
    {
        height = -height;
        dst_argb = dst_argb + (height - 1) * dst_stride_argb;
        dst_stride_argb = -dst_stride_argb;
    }

    for (int y = 0; y < height; ++y) 
    {
        I420ToARGBRow(src_y, src_u, src_v, dst_argb, width);
        dst_argb += dst_stride_argb;
        src_y += src_stride_y;
        if (y & 1) {
            src_u += src_stride_u;
            src_v += src_stride_v;
        }
    }
    return 0;
}
