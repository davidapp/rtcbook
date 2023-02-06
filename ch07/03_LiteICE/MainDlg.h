#pragma once

#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "Base/DXP.h"
#include "Base/DUTF8.h"
#include "Net/DNet.h"
#include "Net/DUDPClient.h"


#define WM_LOG WM_USER+1000
#define WM_UPDATEUI WM_USER+1001
#define WM_LOGINOK WM_USER+1002
#define WM_PROCESS WM_USER+1003
#define WM_UPDATELIST WM_USER+1004


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
        //COMMAND_ID_HANDLER(IDC_CONNECT, OnConnect)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow();
        //m_ip = GetDlgItem(IDC_IP);

        DNet::Init();

        //m_client.Init();
        //m_client.SetSink(this);

        SendMessage(m_hWnd, WM_UPDATEUI, 0, 0);
        return TRUE;
    }

    LRESULT OnUpdateUI(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {

        return 0;
    }

    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        //m_client.UnInit();
        DestroyWindow();
        ::PostQuitMessage(0);
        DNet::UnInit();
        return 0;
    }

    //DUDPClient m_client;
};
