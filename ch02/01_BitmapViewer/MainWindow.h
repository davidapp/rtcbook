#pragma once

#include "DTypes.h"
#include "resource.h"
#include "atlwinx.h"
#include "atlcrack.h"
#include "atlmisc.h"
#include "atlctrls.h"
#include "atlgdi.h"
#include "File/DBmpFile.h"
#include "Base/DFile.h"
#include "Base/DXP.h"
#include "Video/DYUV.h"

class CMainWindow : public CWindowImpl<CMainWindow>
{
public:
    DECLARE_WND_CLASS(L"RTCMainWin")

public:
    CMainWindow()
    {
        m_Height = 0;
        m_Width = 0;
        memset(&m_info, 0, sizeof(DBmpInfo));
    }

    BEGIN_MSG_MAP(CMainWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER(ID_BITMAP_LOAD, OnLoadBitmap)
        COMMAND_ID_HANDLER(ID_BITMAP_LOADANDBLT, OnLoadBitmapAndBlt)
        COMMAND_ID_HANDLER(ID_BITMAP_FILEHEADER, OnFileHeader)
        COMMAND_ID_HANDLER(ID_BITMAP_INFOHEADER, OnInfoHeader)
        COMMAND_ID_HANDLER(ID_BITMAP_SAVEASI420, OnSaveI420)
        COMMAND_ID_HANDLER(ID_YUV_OPENI420FILE, OnOpenI420File)
    END_MSG_MAP()

    //Create
    int OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
        CenterWindow();
        return 0;
    }

    //Size
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

    LRESULT OnLoadBitmap(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"Bitmap Files(*.bmp)\0*.bmp\0", m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            DBmpInfo info = {};
            DBool bOK = DBmpFile::Load(DXP::ws2s(dlg.m_szFileName).c_str(), &info);
            if (!bOK) return 0;

            CClientDC dc(m_hWnd);
            for (int x = 0; x < (int)info.imageWidth; x++) 
            {
                for (int y = 0; y < (int)info.imageHeight; y++) 
                {
                    COLORREF cr = DBmpFile::GetRGB(&info, x, y);
                    dc.SetPixel(x, info.imageHeight - y, cr);
                }
            }

            if (m_info.imageBuf) {
                DBmpFile::Free(&m_info);
            }
            m_info = info;
        }
        return 0;
    }

    LRESULT OnLoadBitmapAndBlt(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"Bitmap Files(*.bmp)\0*.bmp\0", m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            DBmpInfo info = {};
            DBool bOK = DBmpFile::Load(DXP::ws2s(dlg.m_szFileName).c_str(), &info);
            if (!bOK) return 0;

            CClientDC dc(m_hWnd);
            dc.StretchDIBits(0, 0, info.imageWidth, info.imageHeight, 0, 0, 
                info.imageWidth, info.imageHeight, info.imageBuf, (const BITMAPINFO*)info.infoHead,
                DIB_RGB_COLORS, SRCCOPY);

            if (m_info.imageBuf) {
                DBmpFile::Free(&m_info);
            }
            m_info = info;
        }
        return 0;
    }

    LRESULT OnFileHeader(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        std::string str = DBmpFile::DumpBitmapFileHeader(m_info.fileHead);
        std::wstring strMsg = DXP::s2ws(str);
        MessageBox(strMsg.c_str(), L"BITMAPFILEHEADER");
        return 0;
    }

    LRESULT OnInfoHeader(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        std::string str = DBmpFile::DumpBitmapInfoHeader(m_info.infoHead);
        std::wstring strMsg = DXP::s2ws(str);
        MessageBox(strMsg.c_str(), L"BITMAPINFOHEADER");
        return 0;
    }

    LRESULT OnSaveI420(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (m_info.imageBuf == nullptr) {
            MessageBox(L"请先加载bmp文件");
            return 1;
        }
        CString strName;
        strName.Format(L"I420_%dx%d.yuv", m_info.imageWidth, m_info.imageHeight);

        CFileDialog dlg(FALSE, NULL, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"YUV Files(*.yuv)\0*.yuv\0", m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            std::string filepath = DXP::ws2s(dlg.m_szFileName);
            DBuffer bufYUV(m_info.imageWidth * m_info.imageHeight * 3 / 2);
            DByte* pY = bufYUV.GetBuf();
            DByte* pU = pY + bufYUV.GetSize() * 2 / 3;
            DByte* pV = pY + bufYUV.GetSize() * 5 / 6;
            DByte* pRGB = m_info.imageBuf + (m_info.imageHeight - 1) * m_info.imageLineBytes;
            for (DUInt32 i = 0; i < m_info.imageHeight; i++) {
                DYUV::BGR24ToYRow(pRGB, pY, m_info.imageWidth);
                pY += m_info.imageWidth;
                pRGB -= m_info.imageLineBytes;
            }
            pRGB = m_info.imageBuf + (m_info.imageHeight - 1) * m_info.imageLineBytes;
            for (DUInt32 i = 0; i < m_info.imageHeight / 2; i++) 
            {
                DYUV::BGR24ToUVRow(pRGB, m_info.imageLineBytes, pU, pV, m_info.imageWidth);
                pU += m_info.imageWidth / 2;
                pV += m_info.imageWidth / 2;
                pRGB -= m_info.imageLineBytes * 2;
            }
            // only Y is saved
            /*for (DUInt32 i = 0; i < m_info.imageWidth * m_info.imageHeight / 4; i++) {
                *pU++ = 128;
                *pV++ = 128;
            }*/
            DFile file;
            file.OpenFileRW(filepath.c_str(), DFILE_OPEN_ALWAYS);
            file.Write(bufYUV);
            file.Close();
        }
        return 0;
    }

    LRESULT OnOpenI420File(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"YUV Files(*.yuv)\0*.yuv\0", m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            std::string filepath = DXP::ws2s(dlg.m_szFileName);
            DBuffer bufYUV = DFile::FilePath2Buffer(filepath.c_str());
            CString strName = dlg.m_szFileTitle;
            DInt32 start = strName.Find(L"_");
            if (start == -1) {
                MessageBox(L"请用文件名*_<width>x<height>.yuv指定YUV的宽和高");
                return 1;
            }
            DInt32 end = strName.Find(L"x", start);
            if (end == -1) {
                MessageBox(L"请用文件名*_<width>x<height>.yuv指定YUV的宽和高");
                return 1;
            }
            CString strWidth = strName.Mid(start+1, end - start - 1);
            start = end;
            end = strName.Find(L".", start);
            if (end == -1) {
                MessageBox(L"请用文件名*_<width>x<height>.yuv指定YUV的宽和高");
                return 1;
            }
            CString strHeight = strName.Mid(start+1, end - start - 1);
            DUInt32 width = _wtoi(strWidth);
            DUInt32 height = _wtoi(strHeight);
            DByte* pY = bufYUV.GetBuf();
            DByte* pU = pY + bufYUV.GetSize() * 2 / 3;
            DByte* pV = pY + bufYUV.GetSize() * 5 / 6;
            DColor RGB = 0;
            DUInt8* pRGB = (DUInt8*)&RGB;
            CClientDC dc(m_hWnd);
            DUInt32 x = 0, y = 0;
            for (DUInt32 i = 0; i < width * height / 4; i++) {
                DInt32 U = *pU - 128;
                DInt32 V = *pV - 128;
                DInt32 Y1 = *pY;
                DInt32 Y2 = *(pY + 1);
                DInt32 Y3 = *(pY + width);
                DInt32 Y4 = *(pY + width + 1);
                DYUV::YUV2RGB(pRGB, Y1, U, V);
                dc.SetPixel(x, y, DRGB(pRGB[2], pRGB[1], pRGB[0]));
                DYUV::YUV2RGB(pRGB, Y2, U, V);
                dc.SetPixel(x+1, y, DRGB(pRGB[2], pRGB[1], pRGB[0]));
                DYUV::YUV2RGB(pRGB, Y3, U, V);
                dc.SetPixel(x, y+1, DRGB(pRGB[2], pRGB[1], pRGB[0]));
                DYUV::YUV2RGB(pRGB, Y4, U, V);
                dc.SetPixel(x+1, y+1, DRGB(pRGB[2], pRGB[1], pRGB[0]));
                pY+=2;
                pU++;
                pV++;
                x += 2;
                if (x >= width) {
                    x = 0;
                    y += 2;
                    pY += width;
                }
            }
        }
        return 0;
    }

    virtual void OnFinalMessage(HWND /*hWnd*/)
    {
        if (m_info.imageBuf) {
            DBmpFile::Free(&m_info);
        }
        PostQuitMessage(0);
    }

private:
    DInt32 m_Height;
    DInt32 m_Width;
    DBmpInfo m_info;
};

