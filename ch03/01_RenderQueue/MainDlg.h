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
#include "Video/DRenderQueue.h"
#include "Video/DVideoFormat.h"
#include "Video/DVideoI420.h"

DRenderQueue g_localQueue;
DRenderQueue g_remoteQueue;

DVoid* OnFrame(DVideoFrame frame, DVoid* pUserData)
{
    HWND hWnd = (HWND)pUserData;
    BITMAPINFO* pHeader = (BITMAPINFO*)frame.GetUserData();
    delete pHeader;

    if (frame.GetFormat() == DPixelFmt::YUY2)
    {
        DVideoFrame i420frame = DVideoFormat::YUY2ToI420(frame);
        DVideoFrame i420frame_e = DVideoI420::Scale(i420frame, 128, 72, kFilterBox);
        g_localQueue.PushFrame(DVideoI420::Mirror(i420frame_e));
        
        DVideoFrame i420frame_s = DVideoI420::Scale(i420frame, 100, 100, kFilterBox);
        //g_remoteQueue.PushFrame(i420frame_s);
    }
    else {

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
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());

        if (!m_vcap.Init(0, (DVoid*)OnFrame, m_hWnd)) {
            MessageBox(L"没有输出 640*480 的 RGB24 或 YUY2 格式的选项");
        }

        m_localFrame = GetDlgItem(IDC_LOCALVIEW);
        m_remoteFrame = GetDlgItem(IDC_REMOTEVIEW);

        g_localQueue.Start();
        g_remoteQueue.Start();

        CRect rect;
        m_localFrame.GetWindowRect(rect);
        CPoint pos(rect.left, rect.top);
        ::ScreenToClient(m_hWnd, &pos);
        DRect rLocal(pos.x, pos.y, pos.x + rect.Width(), pos.y + rect.Height());
        g_localQueue.Setup(m_hWnd, rLocal);

        m_remoteFrame.GetWindowRect(rect);
        pos.x = rect.left;
        pos.y = rect.top;
        ::ScreenToClient(m_hWnd, &pos);
        DRect rRemote(pos.x, pos.y, pos.x + rect.Width(), pos.y + rect.Height());
        g_remoteQueue.Setup(m_hWnd, rRemote);

        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        ::EndDialog(m_hWnd, wID);
        m_hWnd = NULL;
        PostQuitMessage(0);
        g_localQueue.Stop();
        g_remoteQueue.Stop();
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

    WinDSVideoCapture m_vcap;

    CStatic m_localFrame;
    CStatic m_remoteFrame;
};
