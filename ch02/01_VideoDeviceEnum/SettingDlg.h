#pragma once

#include "atl.h"
#include "atldlgs.h"
#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "Base/DUtil.h"
#include "Base/DUTF8.h"
#include "Video/WinDSCamera.h"


class CSettingDlg : public CDialogImpl<CSettingDlg>, public CMessageFilter
{
public:
    enum { IDD = IDD_DIALOG1 };

    CSettingDlg()
    {
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        return ::IsDialogMessage(m_hWnd, pMsg);
    }

    BEGIN_MSG_MAP(CSettingDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_RANGE_HANDLER(IDOK, IDNO, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());

        m_devlist = GetDlgItem(IDC_COMBO1);
        m_info = GetDlgItem(IDC_EDIT1);

        std::vector<DCameraInfo> devs = WinDSCamera::GetDevices();
        for (const DCameraInfo& dev : devs) {
            std::wstring wstr = DUTF8::UTF8ToUCS2(dev.m_device_name);
            m_devlist.AddString(wstr.c_str());
        }

        if (devs.size() > 0) {
            m_devlist.SetCurSel(0);
            std::wstring infostr = WinDSCamera::GetInfoString(devs[0]);
            m_info.SetWindowText(infostr.c_str());
        }
        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        ::EndDialog(m_hWnd, wID);
        m_hWnd = NULL;
        return 0;
    }

    CComboBox m_devlist;
    CEdit m_info;
};
