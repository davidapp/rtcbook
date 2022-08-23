#pragma once

#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "Base/DUtil.h"
#include "Net/DNet.h"
#include "Net/DTCPServer.h"
#include <locale>
#include <string>

HWND g_NotifyWnd;
DUInt32 g_serverState;
#define WM_UPDATEUI WM_USER+1001

class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter , public DTCPServerSink
{
public:
    enum { IDD = IDD_MAINDIALOG };

    CMainDlg()
    {

    }

    virtual DVoid OnListening(DSocket sock, DUInt16 wPort)
    {
        CString str;
        str.Format(L"Listening at port:%d", wPort);
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        g_serverState = 1;
        ::PostMessage(g_NotifyWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnListenOK(DSocket sock, DUInt16 wPort)
    {
        CString str;
        str.Format(L"Listen OK at port:%d", wPort);
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnListenError(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"Listen Error code:%d reason:%S", code, strReason.c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnNewConn(DSocket sock, DTCPSocket newsock)
    {
        CString str;
        str.Format(L"New connection is coming from %S", newsock.GetName().c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnError(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"Server Error code:%d reason:%S", code, strReason.c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnStop(DSocket sock)
    {
        CString str;
        str.Format(L"Server Stop");
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnPreSend(DSocket sock, DBuffer buffer)
    {
        CString str;
        str.Format(L"OnPreSend");
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
        str.Format(L"OnSendError");
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
        DTCPSocket client;
        client.Attach(sock);
        std::string clientname = client.GetName();
        std::string clientdata = buf.ToHexString();
        str.Format(L"OnRecvBuf from %S : %S", clientname.c_str(), clientdata.c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        client.Detach();
    }

    virtual DVoid OnClose(DSocket sock)
    {
        CString str;
        DTCPSocket client;
        client.Attach(sock);
        std::string clientname = client.GetName();
        str.Format(L"OnClose from %S", clientname.c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnBroken(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        DTCPSocket client;
        client.Attach(sock);
        std::string clientname = client.GetName();
        str.Format(L"OnBroken from %S", clientname.c_str());
        ::PostMessage(g_NotifyWnd, WM_LOG, (WPARAM)NewStr(str), 0);
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
        COMMAND_ID_HANDLER(IDC_INFO, OnInfo)
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

        m_start = GetDlgItem(IDC_START);
        m_stop = GetDlgItem(IDC_STOP);
        m_stop.EnableWindow(FALSE);

        m_port = GetDlgItem(IDC_PORT);
        m_port.SetWindowText(L"1229");
        m_log = GetDlgItem(IDC_LOG);

        g_NotifyWnd = m_hWnd;

        DNet::Init();

        return TRUE;
    }

    LRESULT OnLog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        CString strLog = (LPCWSTR)wParam;
        AppendLog((wchar_t*)strLog.GetString());
        DelStr((WCHAR*)wParam);
        return 0;
    }

    WCHAR* NewStr(CString& str) {
        WCHAR* poststr = new WCHAR[str.GetLength() + 1];
        memcpy_s(poststr, str.GetLength() * 2, str.GetString(), str.GetLength() * 2);
        poststr[str.GetLength()] = 0;
        return poststr;
    }

    void DelStr(WCHAR* str) {
        delete[] str;
    }

    LRESULT OnStartServer(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString strPort;
        m_port.GetWindowText(strPort);
        DUInt16 port = _wtoi(strPort);

        m_server.SetSink(this);
        m_server.Start(port);

        m_start.EnableWindow(FALSE);
        m_stop.EnableWindow(TRUE);
        return 0;
    }

    LRESULT OnStopServer(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        m_server.Stop();
        m_start.EnableWindow(TRUE);
        m_stop.EnableWindow(FALSE);
        return 0;
    }

    LRESULT OnInfo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        //std::string info = DSelectServer::GetInfo();
        //std::wstring winfo = DUtil::s2ws(info);
        //AppendLog((wchar_t*)winfo.c_str());
        return 0;
    }

    void AppendLog(wchar_t* str)
    {
        CString strOld;
        m_log.GetWindowText(strOld);
        CString strLog(str);
        strLog += "\r\n";
        strOld += strLog;
        m_log.SetWindowText(strOld);
        m_log.LineScroll(m_log.GetLineCount());
    }

    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DestroyWindow();
        ::PostQuitMessage(0);
        DNet::UnInit();
        return 0;
    }

    CEdit m_port;
    CEdit m_log;

    CButton m_start;
    CButton m_stop;

    DTCPServer m_server;
};
