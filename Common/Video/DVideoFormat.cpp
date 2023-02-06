#include "DVideoFormat.h"
#include "Base/DXP.h"
#include "Video/DVideoColor.h"


DVideoFrame DVideoFormat::YUY2ToRAW(const DVideoFrame& srcFrame)
{
    DBuffer bufRGB(srcFrame.GetWidth() * srcFrame.GetHeight() * 3);
    DByte* pSRC = srcFrame.GetBuf();
    DByte* pEnd = pSRC + srcFrame.GetSize();
    DByte* pDst = bufRGB.GetBuf();
    while (pSRC != pEnd)
    {
        DYUV2RGB::YUV2RAW_BT601((DUInt8*)pDst, pSRC[0], pSRC[1], pSRC[3]);
        pDst += 3;
        DYUV2RGB::YUV2RAW_BT601((DUInt8*)pDst, pSRC[2], pSRC[1], pSRC[3]);
        pDst += 3;
        pSRC += 4;
    }
    DVideoFrame retFrame(bufRGB.GetBuf(), bufRGB.GetSize(), srcFrame.GetWidth(), srcFrame.GetHeight(),
        DPixelFmt::RAW);
    return retFrame;
}



DVideoFrame DVideoFormat::YUY2ToI420(const DVideoFrame& frameSrc)
{
    DVideoFrame frameRet(frameSrc.GetWidth(), frameSrc.GetHeight(), DPixelFmt::I420);

    DByte* src_yuy2 = frameSrc.GetBuf();
    DInt32 src_stride_yuy2 = frameSrc.GetLineSize();
    DByte* dst_y = frameRet.GetBuf();
    DInt32 dst_stride_y = frameRet.GetLineSize();
    DByte* dst_u = frameRet.GetBuf() + frameRet.GetHeight() * frameRet.GetLineSize();
    DInt32 dst_stride_u = frameRet.GetLineSize() / 2;
    DByte* dst_v = dst_u + frameRet.GetHeight() * frameRet.GetLineSize() / 4;
    DInt32 dst_stride_v = frameRet.GetLineSize() / 2;

    DVideoFormat::YUY2ToI420(src_yuy2, src_stride_yuy2, dst_y, dst_stride_y,
        dst_u, dst_stride_u, dst_v, dst_stride_v, frameSrc.GetWidth(), frameSrc.GetHeight());

    return frameRet;
}

DVideoFrame DVideoFormat::I420ToRAW(const DVideoFrame& srcFrame)
{
    DBuffer bufRGB(srcFrame.GetWidth() * srcFrame.GetHeight() * 3);
    DByte* pDst = bufRGB.GetBuf();

    DByte* pY = srcFrame.GetBuf();
    DByte* pU = pY + srcFrame.GetSize() * 2 / 3;
    DByte* pV = pY + srcFrame.GetSize() * 5 / 6;
    DColor RGB = 0;
    DUInt8* pRGB = (DUInt8*)&RGB;
    DInt32 w = srcFrame.GetWidth();
    DInt32 h = srcFrame.GetHeight();

    DInt32 x = 0, y = 0;
    for (DInt32 i = 0; i < w * h / 4; i++) {
        DInt32 U = *pU;
        DInt32 V = *pV;
        DInt32 Y1 = *pY;
        DInt32 Y2 = *(pY + 1);
        DInt32 Y3 = *(pY + w);
        DInt32 Y4 = *(pY + w + 1);
        DYUV2RGB::YUV2RAW_BT601(pRGB, Y1, U, V);
        pDst[(y * w + x) * 3] = pRGB[0];
        pDst[(y * w + x) * 3 + 1] = pRGB[1];
        pDst[(y * w + x) * 3 + 2] = pRGB[2];
        DYUV2RGB::YUV2RAW_BT601(pRGB, Y2, U, V);
        pDst[(y * w + x + 1) * 3] = pRGB[0];
        pDst[(y * w + x + 1) * 3 + 1] = pRGB[1];
        pDst[(y * w + x + 1) * 3 + 2] = pRGB[2];
        DYUV2RGB::YUV2RAW_BT601(pRGB, Y3, U, V);
        pDst[((y + 1) * w + x) * 3] = pRGB[0];
        pDst[((y + 1) * w + x) * 3 + 1] = pRGB[1];
        pDst[((y + 1) * w + x) * 3 + 2] = pRGB[2];
        DYUV2RGB::YUV2RAW_BT601(pRGB, Y4, U, V);
        pDst[((y + 1) * w + x + 1) * 3] = pRGB[0];
        pDst[((y + 1) * w + x + 1) * 3 + 1] = pRGB[1];
        pDst[((y + 1) * w + x + 1) * 3 + 2] = pRGB[2];
        pY += 2;
        pU++;
        pV++;
        x += 2;
        if (x >= srcFrame.GetWidth()) {
            x = 0;
            y += 2;
            pY += srcFrame.GetWidth();
        }
    }

    DVideoFrame retFrame(bufRGB.GetBuf(), bufRGB.GetSize(), w, h, DPixelFmt::RAW);
    return retFrame;
}

DVideoFrame DVideoFormat::I420ToARGB(const DVideoFrame& frameSrc)
{
    DVideoFrame frameRet(frameSrc.GetWidth(), frameSrc.GetHeight(), DPixelFmt::ARGB);
    return frameRet;
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
