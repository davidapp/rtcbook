#pragma once

#include "stdatl.h"

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

    void UpdateData()
    {

    }

    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
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

        return TRUE;
    }


    LRESULT OnSelChange(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        if (wNotifyCode == BN_CLICKED)
        {
            UpdateData();
        }
        return 0;
    }

    LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        UpdateData();
        return 0;
    }

    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CloseDialog(wID);
        return 0;
    }

    void CloseDialog(int nVal)
    {
        DestroyWindow();
        ::PostQuitMessage(nVal);
    }
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
