#include "DVideoFrame.h"
#include "DYUV.h"

DVideoFrame::DVideoFrame(DByte* data, DUInt32 data_size, DInt32 w, DInt32 h, DPixelFmt fmt) : m_data(data, data_size) 
{
    m_width = w;
    m_height = h;
    m_fmt = fmt;
    m_stride = DefaultStride(m_width, m_fmt);
}

DVideoFrame::DVideoFrame(const DVideoFrame& frame) 
{
    m_width = frame.m_width;
    m_height = frame.m_height;
    m_fmt = frame.m_fmt;
    m_stride = frame.m_stride;
    m_data = frame.m_data;
}

DInt32 DVideoFrame::DefaultStride(DInt32 width, DPixelFmt fmt) 
{
    switch (fmt)
    {
    case DPixelFmt::I420:
    case DPixelFmt::IYUV:
        return width;
    case DPixelFmt::RGB565:
    case DPixelFmt::YUY2:
        return width * 2;
    case DPixelFmt::RGB24:
        return width * 3;
    case DPixelFmt::BGRA:
    case DPixelFmt::ARGB:
        return width * 4;
    default:
        return 0;
    }
}

DVideoFrame* DVideoFrame::YUY2ToRAW(const DVideoFrame* pFrame)
{
    DBuffer bufRGB(pFrame->m_width * pFrame->m_height * 3);
    DByte* pSRC = pFrame->m_data.GetBuf();
    DByte* pEnd = pSRC + pFrame->m_data.GetSize();
    DByte* pDst = bufRGB.GetBuf();
    while (pSRC != pEnd)
    {
        DYUV::YUV2RGB((DUInt8*)pDst, pSRC[0], pSRC[1] - 128, pSRC[3] - 128);
        pDst += 3;
        DYUV::YUV2RGB((DUInt8*)pDst, pSRC[2], pSRC[1] - 128, pSRC[3] - 128);
        pDst += 3;
        pSRC += 4;
    }
    DVideoFrame* pFrameRet = new DVideoFrame(*pFrame);
    pFrameRet->m_data = bufRGB;
    pFrameRet->m_fmt = DPixelFmt::RAW;
    pFrameRet->m_stride = 3;
    return pFrameRet;
}