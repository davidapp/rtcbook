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
        COMMAND_HANDLER(IDC_COMBO1, CBN_SELCHANGE, OnCbnSelchangeCombo1)
        COMMAND_ID_HANDLER(IDC_SHOWINFO, OnShowInfo)
        COMMAND_ID_HANDLER(IDC_DUMPCAPS, OnDumpCaps)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());

        m_devlist = GetDlgItem(IDC_COMBO1);
        m_info = GetDlgItem(IDC_EDIT1);
        m_capList = GetDlgItem(IDC_CAPLIST);

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

    LRESULT OnCbnSelchangeCombo1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        std::vector<DCameraInfo> devs = WinDSCamera::GetDevices();
        DUInt32 index = m_devlist.GetCurSel();
        if (index < devs.size()) {
            std::wstring infostr = WinDSCamera::GetInfoString(devs[index]);
            m_info.SetWindowText(infostr.c_str());
        }
        return 0;
    }

    LRESULT OnShowInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        std::vector<DCameraInfo> devs = WinDSCamera::GetDevices();
        DUInt32 index = m_devlist.GetCurSel();
        if (index < devs.size()) {
            WinDSCamera::ShowSettingDialog(devs[index].m_filter, this->m_hWnd, 0, 0);
        }
        return 0;
    }

    LRESULT OnDumpCaps(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        std::vector<DCameraInfo> devs = WinDSCamera::GetDevices();
        DUInt32 index = m_devlist.GetCurSel();
        if (index < devs.size()) {
            m_capList.ResetContent();

            std::vector<DCameraCaps> caps = WinDSCamera::GetDeviceCaps(devs[index].m_filter);
            for (DUInt32 i = 0; i < caps.size(); i++)
            {
                CString str;
                str.Format(L"%d_%d*%d", i, caps[i].m_width, caps[i].m_height);
                m_capList.AddString(str);
            }
        }
        return 0;
    }
    
    CComboBox m_devlist;
    CEdit m_info;
    CListBox m_capList;
};
