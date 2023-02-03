#include "DVideoFrameUtil.h"
#include "Base/DXP.h"


DVideoFrame DVideoFrameUtil::I420Scale(const DVideoFrame& srcFrame)
{
    DVideoFrame frameRet;
    return frameRet;
}

DVideoFrame DVideoFrameUtil::ScaleTo(DInt32 w, DInt32 h)
{
    DVideoFrame frameRet(w, h, DPixelFmt::I420);
    DByte* src_y = GetData()->buf();
    DInt32 src_stride_y = GetData()->m_lineSize;
    DByte* src_u = GetData()->buf();
    DInt32 src_stride_u = GetData()->m_lineSize / 2;
    DByte* src_v = GetData()->buf();
    DInt32 src_stride_v = GetData()->m_lineSize / 2;

    DByte* dst_y = frameRet.GetBuf();
    DInt32 dst_stride_y = frameRet.GetLineSize();
    DByte* dst_u = frameRet.GetBuf();
    DInt32 dst_stride_u = frameRet.GetLineSize() / 2;
    DByte* dst_v = frameRet.GetBuf();
    DInt32 dst_stride_v = frameRet.GetLineSize() / 2;

    DScale::I420Scale(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v, GetWidth(), GetHeight(),
        dst_y, dst_stride_y, dst_u, dst_stride_u, dst_v, dst_stride_v, w, h, kFilterBox);

    return frameRet;
}

DVideoFrame DVideoFrameUtil::YUY2ToRAW(const DVideoFrame& srcFrame)
{
    DBuffer bufRGB(srcFrame.GetWidth() * srcFrame.GetHeight() * 3);
    DByte* pSRC = srcFrame.GetBuf();
    DByte* pEnd = pSRC + srcFrame.GetSize();
    DByte* pDst = bufRGB.GetBuf();
    while (pSRC != pEnd)
    {
        DYUV::YUV2RGB((DUInt8*)pDst, pSRC[0], pSRC[1] - 128, pSRC[3] - 128);
        pDst += 3;
        DYUV::YUV2RGB((DUInt8*)pDst, pSRC[2], pSRC[1] - 128, pSRC[3] - 128);
        pDst += 3;
        pSRC += 4;
    }
    DVideoFrame retFrame(bufRGB.GetBuf(), bufRGB.GetSize(), srcFrame.GetWidth(), srcFrame.GetHeight(),
        DPixelFmt::RAW);
    return retFrame;
}

DVideoFrame DVideoFrame::YUY2ToI420(const DVideoFrame& frameSrc)
{
    DVideoFrame frameRet(frameSrc.GetWidth(), frameSrc.GetHeight(), DPixelFmt::I420);

    DByte* src_yuy2 = frameSrc.GetBuf();
    DInt32 src_stride_yuy2 = frameSrc.GetLineSize();
    DByte* dst_y = frameRet.GetBuf();
    DInt32 dst_stride_y = frameRet.GetLineSize();
    DByte* dst_u = frameRet.GetBuf() + frameRet.GetHeight() * frameRet.GetLineSize();
    DInt32 dst_stride_u = frameRet.GetLineSize() / 2;
    DByte* dst_v = dst_u + frameRet.GetHeight() * frameRet.GetLineSize() / 2;
    DInt32 dst_stride_v = frameRet.GetLineSize() / 2;

    DYUV::YUY2ToI420(src_yuy2, src_stride_yuy2, dst_y, dst_stride_y,
        dst_u, dst_stride_u, dst_v, dst_stride_v, frameSrc.GetWidth(), frameSrc.GetHeight());

    return frameRet;
}