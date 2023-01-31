#pragma once

#include "atl.h"
#include "atldlgs.h"
#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "atlgdi.h"
#include "COMBridge/WinDSVideoCapture.h"
#include "Video/DVideoFrame.h"
#include "Base/DTimer.h"

DVoid* OnFrame(DVideoFrame frame, DVoid* pFrameData, DVoid* pUserData)
{
    DTimer::Stop(0);
    DTimer::Output(0, DTimeUnit::IN_US);

    HWND hWnd = (HWND)pUserData;
    BITMAPINFO* pHeader = (BITMAPINFO*)pFrameData;

    if (frame.GetFormat() == DPixelFmt::YUY2)
    {
        DVideoFrame frame24 = DVideoFrame::YUY2ToRAW(frame);
        pHeader->bmiHeader.biBitCount = 24;
        pHeader->bmiHeader.biCompression = BI_RGB;
        pHeader->bmiHeader.biSizeImage = frame24.GetSize();

        ::PostMessage(hWnd, WM_ONFRAME, (WPARAM)frame24.GetBuf(), (LPARAM)pHeader);
        frame24.Detach();
    }
    else if (frame.GetFormat() == DPixelFmt::RGB24)
    {
        ::PostMessage(hWnd, WM_ONFRAME, (WPARAM)frame.GetBuf(), (LPARAM)pHeader);
        frame.Detach();
    }

    return nullptr;
}

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
        COMMAND_ID_HANDLER(IDC_START, OnCameraStart)
        COMMAND_ID_HANDLER(IDC_STOP, OnCameraStop)
        MESSAGE_HANDLER(WM_ONFRAME, OnMyFrame)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());
        DTimer::Init();
        if (!m_vcap.Init(0, (DVoid*)OnFrame, m_hWnd)) {
            MessageBox(L"没有输出 640*480 的 RGB24 或 YUY2 格式的选项");
        }

        m_frame = GetDlgItem(IDC_FRAME);
        m_log = GetDlgItem(IDC_LOG);
        m_mirror = GetDlgItem(IDC_MIRROR);
        m_rotate = GetDlgItem(IDC_ROTATE);
        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        ::EndDialog(m_hWnd, wID);
        m_hWnd = NULL;
        PostQuitMessage(0);
        return 0;
    }

    LRESULT OnCameraStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        DTimer::Start(0);
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
        DVideoFrame frame;
        frame.Attach((DByte*)wParam);

        BITMAPINFO* pHeader = (BITMAPINFO*)lParam;
        CClientDC dc(m_hWnd);
        CRect rect;
        m_frame.GetClientRect(&rect);

        if (frame.GetFormat() == DPixelFmt::RGB24)
        {
            dc.StretchDIBits(0, 0, frame.GetWidth(), frame.GetHeight(), 0, 0,
                frame.GetWidth(), frame.GetHeight(), frame.GetBuf(),
                pHeader, DIB_RGB_COLORS, SRCCOPY);
        }
        else if (frame.GetFormat() == DPixelFmt::RAW)
        {
            dc.StretchDIBits(rect.left, rect.top, rect.Width(), rect.Height(), 0, frame.GetHeight(),
                frame.GetWidth(), -frame.GetHeight(), frame.GetBuf(),
                pHeader, DIB_RGB_COLORS, SRCCOPY);
        }

        delete pHeader;
        return 0;
    }


    CEdit m_log;
    CButton m_mirror;
    CComboBox m_rotate;
    CStatic m_frame;

    WinDSVideoCapture m_vcap;
};
