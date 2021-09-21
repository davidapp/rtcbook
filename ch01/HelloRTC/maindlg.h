#pragma once

#include "stdatl.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "Net/DNet.h"
#include "Net/DIOCPServer.h"

class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter
{
public:
    enum { IDD = IDD_MAINDIALOG };

    CMainDlg()
    {
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        return ::IsDialogMessage(m_hWnd, pMsg);
    }

    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_START, OnStartServer)
        COMMAND_ID_HANDLER(IDC_STOP, OnStopServer)
        COMMAND_ID_HANDLER(IDC_CONNECT, OnConnect)
        COMMAND_ID_HANDLER(IDC_DISCONNECT, OnDisconnect)
        MESSAGE_HANDLER(WM_LOG, OnLog)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow();

        // set icons
        HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON1),
            IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
        SetIcon(hIcon, TRUE);
        HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON1),
            IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
        SetIcon(hIconSmall, FALSE);

        // register object for message filtering
        CMessageLoop* pLoop = _Module.GetMessageLoop();
        pLoop->AddMessageFilter(this);

        m_port = GetDlgItem(IDC_PORT);
        m_port.SetWindowText(L"1229");
        m_log = GetDlgItem(IDC_LOG);
        return TRUE;
    }

    LRESULT OnLog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        CString strLog;
        if (wParam == DEVENT_SERVER_READY)
        {
            strLog.Format(L"Server is listened at %hd\r\n", (DUInt16)lParam);
        }
        CString strOld;
        m_log.GetWindowText(strOld);
        strLog += "\r\n";
        strOld += strLog;
        m_log.SetWindowText(strOld);
        return 0;
    }

    LRESULT OnSelChange(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        if (wNotifyCode == BN_CLICKED)
        {
            
        }
        return 0;
    }

    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CloseDialog(wID);
        return 0;
    }

    LRESULT OnStartServer(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString strPort;
        m_port.GetWindowText(strPort);
        DUInt16 port = _wtoi(strPort);
        DIOCPServer::Start(m_hWnd, port);
        return 0;
    }

    LRESULT OnStopServer(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {

        return 0;
    }

    LRESULT OnConnect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {

        return 0;
    }

    LRESULT OnDisconnect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {

        return 0;
    }

    void CloseDialog(int nVal)
    {
        DestroyWindow();
        ::PostQuitMessage(nVal);
    }

    CEdit m_port;
    CEdit m_log;
};
