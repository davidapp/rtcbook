#pragma once

#include "atl.h"
#include "atldlgs.h"
#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "atlgdi.h"
#include "Base/DFile.h"
#include "Video/DVideoFrame.h"
#include "Video/DVideoI420.h"
#include "Video/DVideoFormat.h"

class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter
{
public:
    enum { IDD = IDD_DIALOG1 };

    CMainDlg()
    {
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        return ::IsDialogMessage(m_hWnd, pMsg);
    }

    BEGIN_MSG_MAP(CSettingDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_RANGE_HANDLER(IDOK, IDNO, OnCloseCmd)
        COMMAND_ID_HANDLER(IDC_GDI, OnGDI)
        COMMAND_ID_HANDLER(IDC_OPENGL, OnOpenGL)
        COMMAND_ID_HANDLER(IDC_DIRECTDRAW, OnDirectDraw)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());
        DBuffer bufFrame = DFile::FilePath2Buffer("C:\\Users\\Admin\\Downloads\\I420Frame.vf");
        m_frame.LoadFromBuffer(bufFrame);

        m_destRect.top = 0;
        m_destRect.left = 0;
        m_destRect.bottom = 300;
        m_destRect.right = 300;
        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        ::EndDialog(m_hWnd, wID);
        m_hWnd = NULL;
        PostQuitMessage(0);
        return 0;
    }

    LRESULT OnGDI(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        CDC dc = ::GetDC(m_hWnd);

        DVideoFrame i420frame_e = DVideoI420::Scale(m_frame, m_destRect.Width(), m_destRect.Height(), kFilterBox);
        DVideoFrame frameRaw = DVideoFormat::I420ToRAW(i420frame_e);
        DRect src = DRect(0, 0, i420frame_e.GetWidth(), i420frame_e.GetHeight());
        DBITMAPINFOHEADER* pHrd = frameRaw.NewBMPInfoHeader();
        HBRUSH brush1 = ::CreateSolidBrush(RGB(0, 0, 255));
        RECT logical_rect1 = { 0, 0, m_destRect.Width(), m_destRect.Height() };
        dc.FillRect(&logical_rect1, brush1);
        ::DeleteObject(brush1);
        dc.StretchDIBits(0, 0, m_destRect.Width(), m_destRect.Height(),
            0, src.Height(), src.Width(), -src.Height(), frameRaw.GetBuf(),
            (const BITMAPINFO*)pHrd, DIB_RGB_COLORS, SRCCOPY);
        return 0;
        // Double Buffer
        CDC dc_mem;
        dc_mem.CreateCompatibleDC(dc.m_hDC);
        //dc_mem.SetStretchBltMode(HALFTONE);

        HBITMAP bmp_mem = ::CreateCompatibleBitmap(dc.m_hDC, m_destRect.Width(), m_destRect.Height());
        HGDIOBJ bmp_old = ::SelectObject(dc_mem.m_hDC, bmp_mem);
        HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 255));
        RECT logical_rect = { 0, 0, m_destRect.Width(), m_destRect.Height() };
        ::FillRect(dc_mem, &logical_rect, brush);
        ::DeleteObject(brush);

        dc_mem.StretchDIBits(0, 0, m_destRect.Width(), m_destRect.Height(),
            0, src.Height(), src.Width(), src.Height(), frameRaw.GetBuf(),
            (const BITMAPINFO*)pHrd, DIB_RGB_COLORS, SRCCOPY);

        dc.BitBlt(m_destRect.left, m_destRect.top, m_destRect.Width(), m_destRect.Height(), dc_mem, 0, 0, SRCCOPY);

        ::SelectObject(dc_mem, bmp_old);
        ::DeleteObject(bmp_mem);
        ::DeleteDC(dc_mem);

        delete pHrd;
        return 0;
    }

    LRESULT OnOpenGL(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        return 0;
    }

    LRESULT OnDirectDraw(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        return 0;
    }

    DVideoFrame m_frame;
    DRect m_destRect;
};
