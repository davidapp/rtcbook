#pragma once

#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "Base/DXP.h"
#include "Base/DUTF8.h"
#include "Net/DNet.h"
#include "Net/DTCPClient.h"
#include "DHelloClient.h"

#define WM_LOG WM_USER+1000
#define WM_UPDATEUI WM_USER+1001
#define WM_PROCESS WM_USER+1002


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
        MESSAGE_HANDLER(WM_PROCESS, OnProcess)
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

        m_client.Init();
        m_client.SetSink(this);

        SendMessage(m_hWnd, WM_UPDATEUI, 0, 0);
        return TRUE;
    }

    // DTCPClientSink
    virtual DVoid OnConnecting(DSocket sock, std::string strIP, DUInt16 wPort)
    {
        CString str;
        str.Format(L"Connecting to %S:%d", strIP.c_str(), wPort);
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(m_hWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnConnectOK(DSocket sock)
    {
        CString str;
        str.Format(L"Connected OK");
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(m_hWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnConnectError(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"Connect Error.code:%d reason:%S", code, strReason.c_str());
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(m_hWnd, WM_UPDATEUI, 0, 0);
    }

    // DTCPDataSink
    virtual DVoid OnPreSend(DSocket sock, DBuffer buffer)
    {
        CString str;
        str.Format(L"OnPreSend size:%d data:%S", buffer.GetSize(), buffer.ToHexString().c_str());
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnSendOK(DSocket sock)
    {
        CString str;
        str.Format(L"OnSendOK");
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnSendError(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"OnSendError code:%d reason:%S", code, strReason.c_str());
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnSendTimeout(DSocket sock)
    {
        CString str;
        str.Format(L"OnSendTimeout");
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnRecvBuf(DSocket sock, DBuffer buf)
    {
        CString str;
        str.Format(L"OnRecvBuf size:%d data:%S", buf.GetSize(), buf.ToHexString().c_str());
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);

        ::PostMessage(m_hWnd, WM_PROCESS, (WPARAM)sock, (LPARAM)buf.GetBuf());
        buf.Detach();
    }

    virtual DVoid OnClose(DSocket sock)
    {
        CString str;
        str.Format(L"OnClose");
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(m_hWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnBroken(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"OnBroken");
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(m_hWnd, WM_UPDATEUI, 0, 0);
    }

    LRESULT OnUpdateUI(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        if (m_client.GetState() == DTCPCLIENT_STATE_DISCONNECT) {
            m_connect.EnableWindow();
            m_disconnect.EnableWindow(FALSE);
            m_setname.EnableWindow(FALSE);
            m_name.EnableWindow(FALSE);
            m_send.EnableWindow(FALSE);
            m_input.EnableWindow(FALSE);
            m_info.EnableWindow(FALSE);
            m_ip.EnableWindow();
            m_port.EnableWindow();
        }
        else if (m_client.GetState() == DTCPCLIENT_STATE_CONNECTING) {
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
        else if (m_client.GetState() == DTCPCLIENT_STATE_CONNECTED) {
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

    LRESULT OnProcess(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        DBuffer bufRecv;
        bufRecv.Attach((DByte*)lParam);
        DSocket sock = (DSocket)wParam;
        DHelloClient::HandleRecvBuffer(m_hWnd, sock, bufRecv);
        return 0;
    }

    DWChar* NewStr(CString& str) {
        DWChar* poststr = new DWChar[str.GetLength() + 1];
        memcpy_s(poststr, str.GetLength()*2, str.GetString(), str.GetLength()*2);
        poststr[str.GetLength()] = 0;
        return poststr;
    }

    DVoid DelStr(DWChar* str) {
        delete[] str;
    }

    LRESULT OnLog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        CString strLog = (LPCWSTR)wParam;
        AppendLog((DWChar*)strLog.GetString());
        DelStr((DWChar*)wParam);
        return 0;
    }

    DVoid AppendLog(DWChar* str)
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
        m_client.Connect(DXP::ws2s(std::wstring(strIP)), DXP::Str16ToInt32(std::wstring(strPort)));
        return 0;
    }

    LRESULT OnDisConnect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        m_client.DisConnect();
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
            DUInt32 inputlen = m_sendText.GetLength();
            std::string strU8 = DUTF8::UCS2ToUTF8((DUInt16*)strText.GetString(), strText.GetLength());
            DHelloClient::SendIDText(&m_client, 1, strU8);
        }
        return 0;
    }

    LRESULT OnInfo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DGrowBuffer gb;
        gb.AddUInt32(1, true); // len
        gb.AddUInt8(HELLO_CS_CMD_GETINFO);
        DBuffer bufSend = gb.Finish();
        m_client.Send(bufSend);
        return 0;
    }

    LRESULT OnSetName(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DGrowBuffer gb;
        CString strText;
        m_name.GetWindowText(strText);
        DUInt32 inputlen = strText.GetLength();
        gb.AddUInt32(1 + 4 + inputlen * 2, true); // len
        gb.AddUInt8(HELLO_CS_CMD_SETNAME);
        std::wstring wstr(strText);
        std::string str = DXP::ws2s(wstr);
        gb.AddStringA(str);
        DBuffer bufSend = gb.Finish();
        m_client.Send(bufSend);
        return 0;
    }

    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        m_client.UnInit();
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
    DTCPClient m_client;
};
