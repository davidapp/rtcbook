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

DRenderQueue g_renderQueue;


DVoid* OnFrame(DVideoFrame frame, DVoid* pUserData)
{
    HWND hWnd = (HWND)pUserData;
    BITMAPINFO* pHeader = (BITMAPINFO*)frame.GetUserData();
    delete pHeader;

    if (frame.GetFormat() == DPixelFmt::YUY2)
    {
        DVideoFrame i420frame = DVideoFormat::YUY2ToI420(frame);
        DVideoFrame i420frame_e = DVideoI420::Scale(i420frame, 200, 200, kFilterBox);
        g_renderQueue.PushFrame(D_LOCAL_VIEW, DVideoI420::Mirror(i420frame_e));
        g_renderQueue.PushFrame(D_REMOTE_VIEW, i420frame_e);
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

        CRect rect;
        m_localFrame.GetWindowRect(rect);
        CPoint pos(rect.left, rect.top);
        ::ScreenToClient(m_hWnd, &pos);
        DRect rLocal(pos.x, pos.y, pos.x + rect.Width(), pos.y + rect.Height());
        g_renderQueue.Setup(D_LOCAL_VIEW, m_hWnd, rLocal);

        m_remoteFrame.GetWindowRect(rect);
        pos.x = rect.left;
        pos.y = rect.top;
        ::ScreenToClient(m_hWnd, &pos);
        DRect rRemote(pos.x, pos.y, pos.x + rect.Width(), pos.y + rect.Height());
        g_renderQueue.Setup(D_REMOTE_VIEW, m_hWnd, rRemote);

        g_renderQueue.Start();

        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        ::EndDialog(m_hWnd, wID);
        m_hWnd = NULL;
        PostQuitMessage(0);
        g_renderQueue.Stop();
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
