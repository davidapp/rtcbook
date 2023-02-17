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
#include "Video/DVideoFormat.h"
#include "Video/DVideoColor.h"
#include "Video/DVideoI420.h"
#include "Base/DTime.h"
#include "Base/DFile.h"

#define DEST_WIDTH 300
#define DEST_HEIGHT 200
#define WM_LOG WM_USER+1000

DBool g_mirror = false;
DRotation g_rotate = DRotation::DEGREE_0;
DBool g_dump = false;
DBool g_save = false;

DWChar* NewStr(CString& str) {
    DWChar* poststr = new DWChar[str.GetLength() + 1];
    memcpy_s(poststr, str.GetLength() * 2, str.GetString(), str.GetLength() * 2);
    poststr[str.GetLength()] = 0;
    return poststr;
}

DVoid DelStr(DWChar* str) {
    delete[] str;
}

DVoid* OnFrame(DVideoFrame frame, DVoid* pUserData)
{
    DTimer::Stop(0);
    DTimer::Output(0, DTimeUnit::IN_US);
    HWND hWnd = (HWND)pUserData;
    frame.SetCTS(DTime::GetTimeStamp());

    if (g_dump) {
        std::string strFrame = frame.GetDumpText();
        CString str;
        str.Format(L"%S", strFrame.c_str());
        ::PostMessage(hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        g_dump = false;
    }

    BITMAPINFO* pHeader = (BITMAPINFO*)frame.GetUserData();

    if (frame.GetFormat() == DPixelFmt::YUY2)
    {
        DVideoFrame framei420 = DVideoFormat::YUY2ToI420(frame);
        if (g_save) {
            DBuffer bufFrame = framei420.GetDumpBuffer();
            DFile::Buffer2FilePath(bufFrame, "C:\\Users\\Admin\\Downloads\\I420Frame.vf");
            g_save = false;
        }

        DVideoFrame frame_small = DVideoI420::Scale(framei420, DEST_WIDTH, DEST_HEIGHT, kFilterBox);
        DVideoFrame frame_small_mirror;
        DVideoFrame frame_small_raw;
        if (g_mirror) {
            frame_small_mirror = DVideoI420::Mirror(frame_small);
            DVideoFrame frame_rotate = DVideoI420::Rotate(frame_small_mirror, g_rotate);
            frame_small_raw = DVideoFormat::I420ToRAW(frame_rotate);
        }
        else {
            DVideoFrame frame_rotate = DVideoI420::Rotate(frame_small, g_rotate);
            frame_small_raw = DVideoFormat::I420ToRAW(frame_rotate);
        }

        if (g_rotate == DRotation::DEGREE_90 || g_rotate == DRotation::DEGREE_270) {
            pHeader->bmiHeader.biWidth = DEST_HEIGHT;
            pHeader->bmiHeader.biHeight = DEST_WIDTH;
        }
        else {
            pHeader->bmiHeader.biWidth = DEST_WIDTH;
            pHeader->bmiHeader.biHeight = DEST_HEIGHT;
        }
        pHeader->bmiHeader.biSizeImage = DEST_WIDTH * DEST_HEIGHT * 3;
        pHeader->bmiHeader.biBitCount = 24;
        pHeader->bmiHeader.biCompression = BI_RGB;
        ::PostMessage(hWnd, WM_ONFRAME, (WPARAM)frame_small_raw.GetBuf(), (LPARAM)pHeader);
        frame_small_raw.Detach();
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
        MESSAGE_HANDLER(WM_LOG, OnLog)
        COMMAND_ID_HANDLER(IDC_START, OnCameraStart)
        COMMAND_ID_HANDLER(IDC_STOP, OnCameraStop)
        COMMAND_HANDLER(IDC_COMBO1, CBN_SELCHANGE, OnCbnSelchangeScale)
        COMMAND_HANDLER(IDC_ROTATE, CBN_SELCHANGE, OnCbnSelchangeRotate)
        MESSAGE_HANDLER(WM_ONFRAME, OnMyFrame)
        COMMAND_ID_HANDLER(IDC_MIRROR, OnMirror)
        COMMAND_ID_HANDLER(IDC_DUMP, OnDump)
        COMMAND_ID_HANDLER(IDC_SAVE, OnSave)
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
        m_rotate.AddString(L"ROTATION_0");
        m_rotate.AddString(L"ROTATION_90");
        m_rotate.AddString(L"ROTATION_180");
        m_rotate.AddString(L"ROTATION_270");
        m_rotate.SetCurSel(0);

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

    LRESULT OnMirror(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        g_mirror = !g_mirror;
        return 0;
    }

    LRESULT OnDump(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (!g_dump) g_dump = true;
        return 0;
    }

    LRESULT OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (!g_save) g_save = true;
        return 0;
    }

    LRESULT OnLog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        CString strLog = (LPCWSTR)wParam;
        AppendLog((DWChar*)strLog.GetString());
        DelStr((DWChar*)wParam);
        return 0;
    }

    DVoid AppendLog(DWChar* str)
    {
        CString strOld;
        m_log.GetWindowText(strOld);
        CString strLog(str);
        strLog += "\r\n";
        strOld += strLog;
        m_log.SetWindowText(strOld);
        m_log.LineScroll(m_log.GetLineCount());
    }

    LRESULT OnCbnSelchangeScale(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        int i = m_scale.GetCurSel();
        if (i == 0) scaleMode = DScaleMode::CROP;
        else if (i == 1) scaleMode = DScaleMode::FILL;
        else if (i == 2) scaleMode = DScaleMode::STRETCH;
        return 0;
    }

    LRESULT OnCbnSelchangeRotate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        int i = m_rotate.GetCurSel();
        if (i == 0) g_rotate = DRotation::DEGREE_0;
        else if (i == 1) g_rotate = DRotation::DEGREE_90;
        else if (i == 2) g_rotate = DRotation::DEGREE_180;
        else g_rotate = DRotation::DEGREE_270;
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
            dc.StretchDIBits(dst.left, dst.top, dst.Width(), dst.Height(), 0, src.Height(),
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
