#pragma once

#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "Base/DUtil.h"
#include "Net/DNet.h"
#include "Net/DTCPClient.h"
#include <string>

HWND g_NotifyWnd;
DUInt32 g_connState;
#define WM_UPDATEUI WM_USER+1001

class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter, DTCPClientSink, DTCPDataSink
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
        MESSAGE_HANDLER(WM_LOG, OnLog)
        MESSAGE_HANDLER(WM_UPDATEUI, OnUpdateUI)
        COMMAND_ID_HANDLER(IDC_CONNECT, OnConnect)
        COMMAND_ID_HANDLER(IDC_DISCONNECT, OnDisConnect)
        COMMAND_ID_HANDLER(IDC_SEND, OnSend)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow();
        m_ip = GetDlgItem(IDC_IP);
        m_port = GetDlgItem(IDC_PORT);
        m_name = GetDlgItem(IDC_NAME);
        m_chat = GetDlgItem(IDC_CHAT);
        m_connect = GetDlgItem(IDC_CONNECT);
        m_disconnect = GetDlgItem(IDC_DISCONNECT);
        m_setname = GetDlgItem(IDC_SETNAME);
        m_input = GetDlgItem(IDC_INPUT);
        m_send = GetDlgItem(IDC_SEND);

        m_ip.SetWindowText(L"127.0.0.1");
        m_port.SetWindowText(L"1229");
        m_name.SetWindowTextW(L"David");

        DNet::Init();

        g_NotifyWnd = m_hWnd;
        g_connState = 0;
        m_sock.SetConnSink(this);
        m_sock.SetDataSink(this);

        return TRUE;
    }

    // DTCPClientSink
    virtual DVoid OnConnecting(DTCPClient* sock, std::string strIP, DUInt16 wPort)
    {
        CString str;
        str.Format(L"Connecting to %S:%d", strIP.c_str(), wPort);
        SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)str.GetString(), 0);
        g_connState = 1;
        SendMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnConnectOK(DTCPClient* sock)
    {
        CString str;
        str.Format(L"Connected OK");
        SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)str.GetString(), 0);
        g_connState = 2;
        SendMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnConnectError(DTCPClient* sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"Connect Error.code:%d reason:%S", code, strReason.c_str());
        SendMessage(g_NotifyWnd, WM_LOG, (WPARAM)str.GetString(), 0);
        g_connState = 0;
        SendMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
    }

    // DTCPDataSink
    virtual DVoid OnPreSend(DTCPClient* sock, DBuffer buffer)
    {

    }

    virtual DVoid OnSendOK(DTCPClient* sock)
    {

    }

    virtual DVoid OnSendError(DTCPClient* sock, DUInt32 code, std::string strReason)
    {

    }

    virtual DVoid OnSendTimeout(DTCPClient* sock)
    {

    }

    virtual DVoid OnRecvBuf(DTCPClient* sock, DBuffer buf)
    {

    }

    virtual DVoid OnClose(DTCPClient* sock)
    {

    }

    virtual DVoid OnBroken(DTCPClient* sock, DUInt32 code, std::string strReason)
    {

    }

    LRESULT OnUpdateUI(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        if (g_connState == 0) {
            m_connect.EnableWindow();
            m_disconnect.EnableWindow(0);
            m_setname.EnableWindow(0);
            m_send.EnableWindow(0);
            m_input.EnableWindow(0);
        }
        else if (g_connState == 1) {
            m_connect.EnableWindow(0);
            m_disconnect.EnableWindow(0);
            m_setname.EnableWindow(0);
            m_send.EnableWindow(0);
            m_input.EnableWindow(0);
        }
        else if (g_connState == 2) {
            m_connect.EnableWindow(0);
            m_disconnect.EnableWindow(1);
            m_setname.EnableWindow(1);
            m_send.EnableWindow(1);
            m_input.EnableWindow(1);
        }
        return 0;
    }

    LRESULT OnLog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        CString strLog = (LPCWSTR)wParam;
        AppendLog((wchar_t*)strLog.GetString());
        return 0;
    }

    void AppendLog(wchar_t* str)
    {
        CString strOld;
        m_chat.GetWindowText(strOld);
        CString strLog(str);
        strLog += "\r\n";
        strOld += strLog;
        m_chat.SetWindowText(strOld);
    }

    LRESULT OnConnect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        m_sock.Connect("127.0.0.1", 1229);
        return 0;
    }

    LRESULT OnDisConnect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        m_sock.DisConnect();
        return 0;
    }

    LRESULT OnSend(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString strText;
        m_input.GetWindowText(strText);
        if (strText.GetLength() != 0) {
            m_sendText = strText;
            m_input.SetWindowText(L"");
            DGrowBuffer gb;
            gb.AddUInt32(strText.GetLength() + 5, true);
            gb.AddUInt8(1); // cmd
            std::wstring wstr(strText.GetString());
            gb.AddString(wstr);
            DBuffer bufSend = gb.Finish();
            m_sock.Send(bufSend);
        }
        return 0;
    }

    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DestroyWindow();
        ::PostQuitMessage(0);
        DNet::UnInit();
        return 0;
    }

    CEdit m_ip;
    CEdit m_port;
    CEdit m_name;
    CEdit m_chat;
    CEdit m_input;

    CButton m_connect;
    CButton m_disconnect;
    CButton m_setname;
    CButton m_send;

    CString m_sendText;

    DTCPClient m_sock;
};
