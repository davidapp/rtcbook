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
#include "Base/DUtil.h"

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
            DBool bOK = DBmpFile::Load(DUtil::ws2s(dlg.m_szFileName).c_str(), &info);
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
            DBool bOK = DBmpFile::Load(DUtil::ws2s(dlg.m_szFileName).c_str(), &info);
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
        std::wstring strMsg = DUtil::s2ws(str);
        MessageBox(strMsg.c_str(), L"BITMAPFILEHEADER");
        return 0;
    }

    LRESULT OnInfoHeader(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        std::string str = DBmpFile::DumpBitmapInfoHeader(m_info.infoHead);
        std::wstring strMsg = DUtil::s2ws(str);
        MessageBox(strMsg.c_str(), L"BITMAPINFOHEADER");
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

