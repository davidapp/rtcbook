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
#include "Video/DVideoFrameUtil.h"


DVoid* OnFrame(DVideoFrame frame, DVoid* pUserData)
{
    DTimer::Stop(0);
    DTimer::Output(0, DTimeUnit::IN_US);

    HWND hWnd = (HWND)pUserData;
    BITMAPINFO* pHeader = (BITMAPINFO*)frame.GetUserData();

    if (frame.GetFormat() == DPixelFmt::YUY2)
    {
        DVideoFrame frame24 = DVideoFrameUtil::YUY2ToRAW(frame);
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
        COMMAND_HANDLER(IDC_COMBO1, CBN_SELCHANGE, OnCbnSelchangeScale)
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
        m_scale = GetDlgItem(IDC_SCALE);
        m_scale.AddString(L"SCALE_CROP");
        m_scale.AddString(L"SCALE_FILL");
        m_scale.AddString(L"SCALE_STRETCH");
        m_scale.SetCurSel(0);

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

    LRESULT OnCbnSelchangeScale(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        int i = m_scale.GetCurSel();
        if (i == 0) scaleMode = DScaleMode::CROP;
        else if (i == 1) scaleMode = DScaleMode::FILL;
        else if (i == 2) scaleMode = DScaleMode::STRETCH;
        return 0;
    }

    LRESULT OnMyFrame(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        DVideoFrame frame;
        frame.Attach((DByte*)wParam);

        BITMAPINFO* pHeader = (BITMAPINFO*)lParam;
        CClientDC dc(m_hWnd);
        CRect rect;
        m_frame.GetWindowRect(rect);
        CPoint pos(rect.left, rect.top);
        ::ScreenToClient(m_hWnd, &pos);
        CRect src = CRect(0, 0, frame.GetWidth(), frame.GetHeight());
        CRect dst = CRect(0, 0, rect.Width(), rect.Height());
        if (scaleMode == DScaleMode::CROP) {
            int crop_width = DMin(src.Width(), dst.Width() * src.Height() / dst.Height());
            int crop_height = DMin(src.Height(), dst.Height() * src.Width() / dst.Width());
            int offset_x = (src.Width() - crop_width) / 2;
            offset_x = int(offset_x / 2) * 2;
            int offset_y = (src.Height() - crop_height) / 2;
            src.left = offset_x;
            src.right = src.left + crop_width;
            src.top = offset_y;
            src.bottom = offset_y + crop_height;
            dst.left = pos.x;
            dst.top = pos.y;
            dst.right = pos.x + rect.Width();
            dst.bottom = pos.y + rect.Height();
        }
        else if (scaleMode == DScaleMode::FILL) {
            uint32_t target_width = DMin(dst.Width(), src.Width() * dst.Height() / src.Height());
            uint32_t target_height = DMin(dst.Height(), dst.Width() * src.Height() / src.Width());
            CRect winRect(pos.x, pos.y, pos.x + rect.Width(), pos.y + rect.Height());
            dc.FillRect(&winRect, 0);
            dst.left = pos.x;
            dst.top = pos.y;
            dst.right = pos.x + target_width;
            dst.bottom = pos.y + target_height;
        }
        else if (scaleMode == DScaleMode::STRETCH) {
            dst.left = pos.x;
            dst.top = pos.y;
            dst.right = pos.x + rect.Width();
            dst.bottom = pos.y + rect.Height();
        }

        if (frame.GetFormat() == DPixelFmt::RGB24)
        {
            dc.StretchDIBits(dst.left, dst.top, dst.Width(), dst.Height(), 0, 0,
                frame.GetWidth(), frame.GetHeight(), frame.GetBuf(),
                pHeader, DIB_RGB_COLORS, SRCCOPY);
        }
        else if (frame.GetFormat() == DPixelFmt::RAW)
        {
            dc.StretchDIBits(dst.left, dst.top, dst.Width(), dst.Height(), src.left, src.top + src.Height(),
                src.Width(), -src.Height(), frame.GetBuf(),
                pHeader, DIB_RGB_COLORS, SRCCOPY);
        }

        delete pHeader;
        return 0;
    }


    CEdit m_log;
    CComboBox m_scale;
    CButton m_mirror;
    CComboBox m_rotate;
    CStatic m_frame;

    WinDSVideoCapture m_vcap;
    DScaleMode scaleMode = DScaleMode::CROP;
};
