#pragma once

#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "Base/DUtil.h"
#include "Net/DNet.h"
#include "Net/DTCPClient.h"
#include <string>

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
        m_ip.SetWindowText(L"127.0.0.1");
        m_port.SetWindowText(L"1229");
        m_name.SetWindowTextW(L"David");

        m_sock.SetConnSink(this);
        m_sock.SetDataSink(this);
        return TRUE;
    }

    // DTCPClientSink
    virtual DVoid OnConnecting(DTCPClient* sock, std::string strIP, DUInt16 wPort)
    {

    }

    virtual DVoid OnConnectOK(DTCPClient* sock)
    {

    }

    virtual DVoid OnConnectError(DTCPClient* sock, DUInt32 code, std::string strReason)
    {

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

    LRESULT OnLog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        CString strLog;
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
        m_sock.SyncConnect("127.0.0.1", 1229);
        return 0;
    }

    LRESULT OnDisConnect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        m_sock.DisConnect();
        return 0;
    }

    LRESULT OnSend(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DBuffer buf(16);
        buf.FillWithRandom();
        m_sock.SyncSend(buf);
        return 0;
    }

    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DestroyWindow();
        ::PostQuitMessage(0);
        return 0;
    }

    CEdit m_ip;
    CEdit m_port;
    CEdit m_name;
    CEdit m_chat;

    CButton m_connect;
    CButton m_disconnect;

    DTCPClient m_sock;
};
