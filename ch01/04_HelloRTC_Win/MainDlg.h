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
#define WM_UPDATEUI WM_USER+1001

#define TCP_DEMO_CMD_SEND 1
#define TCP_DEMO_CMD_INFO 2
#define TCP_DEMO_CMD_NAME 3


class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter, DTCPClientSink
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
        COMMAND_ID_HANDLER(IDC_INFO, OnInfo)
        COMMAND_ID_HANDLER(IDC_SETNAME, OnSetName)
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
        m_info = GetDlgItem(IDC_INFO);

        m_ip.SetWindowText(L"127.0.0.1");
        m_port.SetWindowText(L"1229");
        m_name.SetWindowText(L"David");

        DNet::Init();

        g_NotifyWnd = m_hWnd;

        m_sock.Init();
        m_sock.SetSink(this);

        SendMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
        return TRUE;
    }

    // DTCPClientSink
    virtual DVoid OnConnecting(DSocket sock, std::string strIP, DUInt16 wPort)
    {
        CString str;
        str.Format(L"Connecting to %S:%d", strIP.c_str(), wPort);
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnConnectOK(DSocket sock)
    {
        CString str;
        str.Format(L"Connected OK");
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnConnectError(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"Connect Error.code:%d reason:%S", code, strReason.c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
    }

    // DTCPDataSink
    virtual DVoid OnPreSend(DSocket sock, DBuffer buffer)
    {
        CString str;
        str.Format(L"OnPreSend size:%d data:%S", buffer.GetSize(), buffer.ToHexString().c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnSendOK(DSocket sock)
    {
        CString str;
        str.Format(L"OnSendOK");
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnSendError(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"OnSendError code:%d reason:%S", code, strReason.c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnSendTimeout(DSocket sock)
    {
        CString str;
        str.Format(L"OnSendTimeout");
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnRecvBuf(DSocket sock, DBuffer buf)
    {
        CString str;
        str.Format(L"OnRecvBuf size:%d data:%S", buf.GetSize(), buf.ToHexString().c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnClose(DSocket sock)
    {
        CString str;
        str.Format(L"OnClose");
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnBroken(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"OnBroken");
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
    }

    LRESULT OnUpdateUI(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        if (m_sock.GetState() == CONN_STATE_DISCONNECT) {
            m_connect.EnableWindow();
            m_disconnect.EnableWindow(FALSE);
            m_setname.EnableWindow(FALSE);
            m_name.EnableWindow(FALSE);
            m_send.EnableWindow(FALSE);
            m_input.EnableWindow(FALSE);
            m_info.EnableWindow();
            m_ip.EnableWindow();
            m_port.EnableWindow();
        }
        else if (m_sock.GetState() == CONN_STATE_CONNECTING) {
            m_connect.EnableWindow(FALSE);
            m_disconnect.EnableWindow();
            m_setname.EnableWindow(FALSE);
            m_name.EnableWindow(FALSE);
            m_send.EnableWindow(FALSE);
            m_input.EnableWindow(FALSE);
            m_info.EnableWindow();
            m_ip.EnableWindow(FALSE);
            m_port.EnableWindow(FALSE);
        }
        else if (m_sock.GetState() == CONN_STATE_CONNECTED) {
            m_connect.EnableWindow(FALSE);
            m_disconnect.EnableWindow();
            m_setname.EnableWindow();
            m_name.EnableWindow();
            m_send.EnableWindow();
            m_input.EnableWindow();
            m_info.EnableWindow();
            m_ip.EnableWindow(FALSE);
            m_port.EnableWindow(FALSE);
        }
        return 0;
    }

    WCHAR* NewStr(CString& str) {
        WCHAR* poststr = new WCHAR[str.GetLength() + 1];
        memcpy_s(poststr, str.GetLength()*2, str.GetString(), str.GetLength()*2);
        poststr[str.GetLength()] = 0;
        return poststr;
    }

    void DelStr(WCHAR* str) {
        delete[] str;
    }

    LRESULT OnLog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        CString strLog = (LPCWSTR)wParam;
        AppendLog((wchar_t*)strLog.GetString());
        DelStr((WCHAR*)wParam);
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
        m_chat.LineScroll(m_chat.GetLineCount());
    }

    LRESULT OnConnect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString strIP;
        m_ip.GetWindowText(strIP);
        CString strPort;
        m_port.GetWindowText(strPort);
        m_sock.Connect(DUtil::ws2s(std::wstring(strIP)), DUtil::Str16ToInt32(std::wstring(strPort)));
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
        DUInt32 inputlen = strText.GetLength();
        if (inputlen != 0) {
            m_sendText = strText;
            m_input.SetWindowText(L"");
            SendText();
        }
        return 0;
    }

    void SendText() {
        DGrowBuffer gb;
        DUInt32 inputlen = m_sendText.GetLength();
        gb.AddUInt32(1 + 4 + inputlen * 2, true); // len
        gb.AddUInt8(TCP_DEMO_CMD_SEND);
        std::wstring wstr(m_sendText);
        gb.AddString(wstr);
        DBuffer bufSend = gb.Finish();
        m_sock.Send(bufSend);
    }

    LRESULT OnInfo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DGrowBuffer gb;
        gb.AddUInt32(1, true); // len
        gb.AddUInt8(TCP_DEMO_CMD_INFO);
        DBuffer bufSend = gb.Finish();
        m_sock.Send(bufSend);
        return 0;
    }

    LRESULT OnSetName(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DGrowBuffer gb;
        CString strText;
        m_name.GetWindowText(strText);
        DUInt32 inputlen = strText.GetLength();
        gb.AddUInt32(1 + 4 + inputlen * 2, true); // len
        gb.AddUInt8(TCP_DEMO_CMD_NAME);
        std::wstring wstr(strText);
        gb.AddString(wstr);
        DBuffer bufSend = gb.Finish();
        m_sock.Send(bufSend);
        return 0;
    }

    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        m_sock.UnInit();
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
    CButton m_info;

    CString m_sendText;
    DTCPClient m_sock;
};
