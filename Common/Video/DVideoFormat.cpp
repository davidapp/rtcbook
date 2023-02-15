#include "DVideoFormat.h"
#include "Base/DXP.h"
#include "Video/DVideoColor.h"


DVideoFrame DVideoFormat::YUY2ToRAW(const DVideoFrame& srcFrame)
{
    DInt32 src_width = srcFrame.GetWidth();
    DInt32 src_height = srcFrame.GetHeight();
    DVideoFrame retFrame(src_width, src_height, DPixelFmt::RAW);
    DInt32 dst_linesize = retFrame.GetLineSize();

    DByte* pSRC = srcFrame.GetBuf();
    DByte* pEnd = pSRC + srcFrame.GetSize();
    DByte* pDst = retFrame.GetBuf();
    DInt32 lineCount = 0;
    while (pSRC != pEnd)
    {
        DYUV2RGB::YUV2RAW_BT601((DUInt8*)pDst, pSRC[0], pSRC[1], pSRC[3]);
        pDst += 3;
        DYUV2RGB::YUV2RAW_BT601((DUInt8*)pDst, pSRC[2], pSRC[1], pSRC[3]);
        pDst += 3;
        pSRC += 4;
        lineCount += 3;
        if ((dst_linesize - lineCount) < 3) { // for padding
            pDst += (dst_linesize - lineCount);
            lineCount = 0;
        }
    }
    return retFrame;
}

DVideoFrame DVideoFormat::YUY2ToI420(const DVideoFrame& frameSrc)
{
    DInt32 src_width = frameSrc.GetWidth();
    DInt32 src_height = frameSrc.GetHeight();
    DVideoFrame frameRet(src_width, src_height, DPixelFmt::I420);

    DByte* src_yuy2 = frameSrc.GetBuf();
    DInt32 src_stride_yuy2 = frameSrc.GetLineSize();
    DByte* dst_y = frameRet.GetBuf();
    DInt32 dst_stride_y = src_width;
    DByte* dst_u = frameRet.GetBuf() + src_height * src_width;
    DInt32 dst_stride_u = (src_width % 2 == 0) ? src_width / 2 : src_width / 2 + 1;
    DInt32 dst_height_u = (src_height % 2 == 0) ? src_height / 2 : src_height / 2 + 1;
    DByte* dst_v = dst_u + dst_stride_u * dst_height_u;
    DInt32 dst_stride_v = dst_stride_u;

    DVideoFormat::YUY2ToI420(src_yuy2, src_stride_yuy2, dst_y, dst_stride_y,
        dst_u, dst_stride_u, dst_v, dst_stride_v, src_width, src_height);

    return frameRet;
}

DVideoFrame DVideoFormat::RAWToI420(const DVideoFrame& frameSrc)
{
    DInt32 src_width = frameSrc.GetWidth();
    DInt32 src_height = frameSrc.GetHeight();
    DVideoFrame frameRet(src_width, src_height, DPixelFmt::I420);


    return frameRet;
}

DVideoFrame DVideoFormat::I420ToRAW(const DVideoFrame& frameSrc)
{
    DInt32 src_width = frameSrc.GetWidth();
    DInt32 src_height = frameSrc.GetHeight();
    DVideoFrame retFrame(src_width, src_height, DPixelFmt::RAW);

    DByte* pY = frameSrc.GetBuf();
    DByte* pU = pY + src_width * src_height;
    DInt32 dst_u_line = (src_width % 2) == 0 ? src_width / 2 : src_width / 2 + 1;
    DInt32 dst_u_height = (src_height % 2) == 0 ? src_height / 2 : src_height / 2 + 1;
    DByte* pV = pU + dst_u_line * dst_u_height;
    DByte* pDst = retFrame.GetBuf();
    DInt32 dst_linesize = retFrame.GetLineSize();
    DColor RGB = 0;
    DUInt8* pRGB = (DUInt8*)&RGB;

    DInt32 U, V, Y1, Y2, Y3, Y4;
    DInt32 x, y;
    for (y = 0; y < src_height; y += 2) {
        for (x = 0; x < src_width; x += 2) {
            U = *pU;
            V = *pV;
            Y1 = *pY;
            Y2 = *(pY + 1);
            Y3 = *(pY + src_width);
            Y4 = *(pY + src_width + 1);
            DYUV2RGB::YUV2RAW_BT601(pRGB, Y1, U, V);
            pDst[(y * src_width + x) * 3] = pRGB[0];
            pDst[(y * src_width + x) * 3 + 1] = pRGB[1];
            pDst[(y * src_width + x) * 3 + 2] = pRGB[2];
            DYUV2RGB::YUV2RAW_BT601(pRGB, Y2, U, V);
            pDst[(y * src_width + x + 1) * 3] = pRGB[0];
            pDst[(y * src_width + x + 1) * 3 + 1] = pRGB[1];
            pDst[(y * src_width + x + 1) * 3 + 2] = pRGB[2];
            DYUV2RGB::YUV2RAW_BT601(pRGB, Y3, U, V);
            pDst[((y + 1) * src_width + x) * 3] = pRGB[0];
            pDst[((y + 1) * src_width + x) * 3 + 1] = pRGB[1];
            pDst[((y + 1) * src_width + x) * 3 + 2] = pRGB[2];
            DYUV2RGB::YUV2RAW_BT601(pRGB, Y4, U, V);
            pDst[((y + 1) * src_width + x + 1) * 3] = pRGB[0];
            pDst[((y + 1) * src_width + x + 1) * 3 + 1] = pRGB[1];
            pDst[((y + 1) * src_width + x + 1) * 3 + 2] = pRGB[2];
            pY += 2;
            pU++;
            pV++;
        }
        if (src_width % 2 != 0) {
            U = *pU;
            V = *pV;
            Y1 = *pY;
            Y3 = *(pY + src_width);
            DYUV2RGB::YUV2RAW_BT601(pRGB, Y1, U, V);
            pDst[(y * src_width + x) * 3] = pRGB[0];
            pDst[(y * src_width + x) * 3 + 1] = pRGB[1];
            pDst[(y * src_width + x) * 3 + 2] = pRGB[2];
            DYUV2RGB::YUV2RAW_BT601(pRGB, Y3, U, V);
            pDst[((y + 1) * src_width + x) * 3] = pRGB[0];
            pDst[((y + 1) * src_width + x) * 3 + 1] = pRGB[1];
            pDst[((y + 1) * src_width + x) * 3 + 2] = pRGB[2];
            pY++;
            pU++;
            pV++;
        }
        pY += src_width;
    }
    if (src_height % 2 != 0) {
        for (x = 0; x < src_width - 1; x += 2) {
            U = *pU;
            V = *pV;
            Y1 = *pY;
            Y2 = *(pY + 1);
            DYUV2RGB::YUV2RAW_BT601(pRGB, Y1, U, V);
            pDst[(y * src_width + x) * 3] = pRGB[0];
            pDst[(y * src_width + x) * 3 + 1] = pRGB[1];
            pDst[(y * src_width + x) * 3 + 2] = pRGB[2];
            DYUV2RGB::YUV2RAW_BT601(pRGB, Y2, U, V);
            pDst[(y * src_width + x + 1) * 3] = pRGB[0];
            pDst[(y * src_width + x + 1) * 3 + 1] = pRGB[1];
            pDst[(y * src_width + x + 1) * 3 + 2] = pRGB[2];
            pY += 2;
            pU++;
            pV++;
        }
        if (src_width % 2 != 0) {
            U = *pU;
            V = *pV;
            Y1 = *pY;
            DYUV2RGB::YUV2RAW_BT601(pRGB, Y1, U, V);
            pDst[(y * src_width + x) * 3] = pRGB[0];
            pDst[(y * src_width + x) * 3 + 1] = pRGB[1];
            pDst[(y * src_width + x) * 3 + 2] = pRGB[2];
            pY++;
            pU++;
            pV++;
        }
    }
    return retFrame;
}

DVideoFrame DVideoFormat::I420ToARGB(const DVideoFrame& frameSrc)
{
    DInt32 src_width = frameSrc.GetWidth();
    DInt32 src_height = frameSrc.GetHeight();
    DVideoFrame frameRet(frameSrc.GetWidth(), frameSrc.GetHeight(), DPixelFmt::ARGB);
    return frameRet;
}


DVoid YUY2ToYRow(const DByte* src_yuy2, DByte* dst_y, DInt32 width)
{
    // [Y0,U0,Y1,V0]
    for (DInt32 x = 0; x < width - 1; x += 2) 
    {
        dst_y[x] = src_yuy2[0];
        dst_y[x + 1] = src_yuy2[2];
        src_yuy2 += 4;
    }
    // [Y4,U4,0,V4]
    if (width & 1)
    {
        dst_y[width - 1] = src_yuy2[0];
    }
}

DVoid YUY2ToUVRow(const DByte* src_yuy2, DInt32 src_stride_yuy2, DByte* dst_u, DByte* dst_v, DInt32 width)
{
    for (DInt32 x = 0; x < width; x += 2)
    {
        dst_u[0] = (src_yuy2[1] + src_yuy2[src_stride_yuy2 + 1] + 1) >> 1; // 本行的 U + 下行的 U 平均
        dst_v[0] = (src_yuy2[3] + src_yuy2[src_stride_yuy2 + 3] + 1) >> 1; // 本行的 V + 下行的 V 平均
        src_yuy2 += 4;
        dst_u += 1;
        dst_v += 1;
    }
}

DInt32 DVideoFormat::YUY2ToI420(const DByte* src_yuy2, DInt32 src_stride_yuy2,
    DByte* dst_y, DInt32 dst_stride_y, DByte* dst_u, DInt32 dst_stride_u, DByte* dst_v, DInt32 dst_stride_v,
    DInt32 width, DInt32 height)
{
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

DVoid I420ToARGBRow(const DByte* src_y, const DByte* src_u, const DByte* src_v, DByte* rgb_buf, DInt32 width)
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

DInt32 DVideoFormat::I420ToARGB(const DByte* src_y, DInt32 src_stride_y,
    const DByte* src_u, DInt32 src_stride_u,
    const DByte* src_v, DInt32 src_stride_v,
    DByte* dst_argb, DInt32 dst_stride_argb,
    DInt32 width, DInt32 height)
{
    if (!src_y || !src_u || !src_v || !dst_argb || width <= 0 || height == 0) {
        return -1;
    }

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
