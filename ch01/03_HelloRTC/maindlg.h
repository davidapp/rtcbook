#pragma once

#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "Base/DXP.h"
#include "Base/DUTF8.h"
#include "Net/DNet.h"
#include "Net/DTCPServer.h"
#include "DHelloServer.h"

#define WM_LOG WM_USER+1000
#define WM_UPDATEUI WM_USER+1001
#define WM_UPDATE_LIST WM_USER+1002

#define DEFAULT_PORT L"1229"

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
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);

        ::PostMessage(m_hWnd, WM_UPDATEUI, 0, 0);
    }

    virtual DVoid OnListenOK(DSocket sock, DUInt16 wPort)
    {
        CString str;
        str.Format(L"Listen OK at port:%d", wPort);
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnListenError(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"Listen Error code:%d reason:%S", code, strReason.c_str());
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnNewConn(DSocket sock, DSocket newsock)
    {
        DTCPSocket tcpsock(newsock);
        CString str;
        str.Format(L"New connection is coming from %S", tcpsock.GetName().c_str());
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        ::PostMessage(m_hWnd, WM_UPDATE_LIST, 0, 0);
    }

    virtual DVoid OnError(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        str.Format(L"Server Error code:%d reason:%S", code, strReason.c_str());
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnStop(DSocket sock)
    {
        CString str;
        str.Format(L"Server Stop");
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
    }

    virtual DVoid OnRecvBuf(DSocket sock, DBuffer buf)
    {
        DHelloServer::Process(&(this->m_server), sock, buf);
        ::PostMessage(m_hWnd, WM_UPDATE_LIST, 0, 0);
    }

    virtual DVoid OnClose(DSocket sock)
    {
        CString str;
        DTCPSocket client;
        client.Attach(sock);
        std::string clientname = client.GetName();
        str.Format(L"OnClose from %S", clientname.c_str());
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        client.Detach();

        ::PostMessage(m_hWnd, WM_UPDATE_LIST, 0, 0);
    }

    virtual DVoid OnBroken(DSocket sock, DUInt32 code, std::string strReason)
    {
        CString str;
        DTCPSocket client;
        client.Attach(sock);
        std::string clientname = client.GetName();
        str.Format(L"OnBroken from %S", clientname.c_str());
        ::PostMessage(m_hWnd, WM_LOG, (WPARAM)NewStr(str), 0);
        client.Detach();

        ::PostMessage(m_hWnd, WM_UPDATE_LIST, 0, 0);
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
        MESSAGE_HANDLER(WM_UPDATE_LIST, OnUpdateList)
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
        m_port.SetWindowText(DEFAULT_PORT);
        m_log = GetDlgItem(IDC_LOG);

        m_userlist = GetDlgItem(IDC_USERLIST);

        DNet::Init();

        return TRUE;
    }

    LRESULT OnLog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        CString strLog = (LPCWSTR)wParam;
        AppendLog((DWChar*)strLog.GetString());
        DelStr((WCHAR*)wParam);
        return 0;
    }

    DWChar* NewStr(CString& str) {
        DWChar* poststr = new DWChar[str.GetLength() + 1];
        memcpy_s(poststr, str.GetLength() * 2, str.GetString(), str.GetLength() * 2);
        poststr[str.GetLength()] = 0;
        return poststr;
    }

    DVoid DelStr(DWChar* str) {
        delete[] str;
    }

    LRESULT OnUpdateList(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        RefreshUserList();
        return 0;
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
        std::string info = m_server.GetServerInfo();
        std::wstring winfo = DUTF8::UTF8ToUCS2(info);
        AppendLog((DWChar*)winfo.c_str());
        return 0;
    }

    DVoid AppendLog(DWChar* str)
    {
        CString strOld;
        m_log.GetWindowText(strOld);
        CString strLog(str);
        strLog += "\r\n";
        strOld += strLog;
        m_log.SetWindowText(strOld);
        m_log.LineScroll(m_log.GetLineCount());
    }

    DVoid RefreshUserList()
    {
        DInt32 Count = m_userlist.GetCount();
        for (DInt32 i = Count - 1; i >= 0; i--)
        {
            m_userlist.DeleteString(i);
        }
        DUInt32 nCount = m_server.GetClientCount();
        for (DUInt32 i = 0; i < nCount; i++)
        {
            DClientData data = m_server.GetClient(i);
            std::string item = data.m_name;
            item += "(";
            item += DXP::UInt32ToStr(data.m_id);
            item += ")";
            std::wstring wstr = DUTF8::UTF8ToUCS2(item);
            m_userlist.AddString(wstr.c_str());
        }
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

    CListBox m_userlist;

    DTCPServer m_server;
};
