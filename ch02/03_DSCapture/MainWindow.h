#pragma once

#include "DTypes.h"
#include "resource.h"
#include "atlwinx.h"
#include "atlcrack.h"
#include "atlmisc.h"
#include "atlctrls.h"
#include "atlgdi.h"
#include "Video/WinDSVideoCapture.h"
#include "Video/DVideoFrame.h"
#include "File/DBmpFile.h"
#include "Base/DFile.h"
#include "Video/DYUV.h"

DVoid* OnFrame(DVoid* pVFrame, DVoid* pFrameData, DVoid* pUserData)
{
    HWND hWnd = (HWND)pUserData;
    DVideoFrame* pFrame = (DVideoFrame*)pVFrame;
    BITMAPINFO* pHeader = (BITMAPINFO*)pFrameData;

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
    pHeader->bmiHeader.biBitCount = 3;
    pHeader->bmiHeader.biCompression = BI_RGB;
    pFrame->m_data = bufRGB;
    pFrame->m_fmt = DPixelFmt::RGB24;
    pFrame->m_stride = 3;
    pHeader->bmiHeader.biSizeImage = bufRGB.GetSize();

    DBuffer bufFile = DBmpFile::Make24BitBitmap(pFrame->m_width, pFrame->m_height, bufRGB);
    DFile file;
    file.OpenFileRW("C:\\Users\\david_ms09i0l\\1.bmp", DFILE_OPEN_ALWAYS);
    file.Write(bufFile);
    file.Close();

    ::PostMessage(hWnd, WM_ONFRAME, (WPARAM)pFrame, (LPARAM)pFrameData);
    return nullptr;
}

class CMainWindow : public CWindowImpl<CMainWindow>
{
public:
    DECLARE_WND_CLASS(L"RTCMainWin")

public:
    CMainWindow()
    {
        m_Height = 0;
        m_Width = 0;
    }

    BEGIN_MSG_MAP(CMainWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER(ID_CAMERA_START, OnCameraStart)
        COMMAND_ID_HANDLER(ID_CAMERA_STOP, OnCameraStop)
        MESSAGE_HANDLER(WM_ONFRAME, OnMyFrame)
    END_MSG_MAP()

    int OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
        CenterWindow();
        if (!m_vcap.Init(0, (DVoid*)OnFrame, m_hWnd)) {
            MessageBox(L"没有输出 640*480 的 RGB24 或 YUY2 格式的选项");
        }
        return 0;
    }

    void OnSize(UINT nType, CSize size)
    {
        m_Width = size.cx;
        m_Height = size.cy;
    }

    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        CPaintDC dc(m_hWnd);
        RECT r;
        GetClientRect(&r);

        return 0;
    }

    LRESULT OnCameraStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        m_vcap.Start();
        return 0;
    }

    LRESULT OnCameraStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        m_vcap.Stop();
        return 0;
    }

    LRESULT OnMyFrame(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        DVideoFrame* pFrame = (DVideoFrame*)wParam;
        BITMAPINFO* pHeader = (BITMAPINFO*)lParam;
        CClientDC dc(m_hWnd);

        if (pFrame->m_fmt == DPixelFmt::RGB24)
        {
            dc.StretchDIBits(0, 0, pFrame->m_width, pFrame->m_height, 0, 0,
                pFrame->m_width, pFrame->m_height, pFrame->m_data.GetBuf(),
                pHeader, DIB_RGB_COLORS, SRCCOPY);
        }
        else if (pFrame->m_fmt == DPixelFmt::YUY2) 
        {
            DBuffer bufRGB(pFrame->m_width * pFrame->m_height * 3);
            DByte* pSRC = pFrame->m_data.GetBuf();
            DByte* pEnd = pSRC + pFrame->m_data.GetSize();
            DByte* pDst = bufRGB.GetBuf();
            while (pSRC != pEnd)
            {
                DYUV::YUV2RGB((DUInt8*)pDst, pSRC[0], pSRC[1]-128, pSRC[3]-128);
                pDst += 3;
                DYUV::YUV2RGB((DUInt8*)pDst, pSRC[2], pSRC[1]-128, pSRC[3]-128);
                pDst += 3;
                pSRC += 4;
            }
            pHeader->bmiHeader.biBitCount = 3;
            pHeader->bmiHeader.biCompression = BI_RGB;
            pHeader->bmiHeader.biSizeImage = bufRGB.GetSize();

            dc.StretchDIBits(0, 0, pFrame->m_width, pFrame->m_height, 0, 0,
                pFrame->m_width, pFrame->m_height, bufRGB.GetBuf(),
                pHeader, DIB_RGB_COLORS, SRCCOPY);
        }
        delete pFrame;
        delete pHeader;
        return 0;
    }

    virtual void OnFinalMessage(HWND /*hWnd*/)
    {
        PostQuitMessage(0);
    }

private:
    DInt32 m_Height;
    DInt32 m_Width;
    WinDSVideoCapture m_vcap;
};

