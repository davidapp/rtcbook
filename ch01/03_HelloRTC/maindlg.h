#pragma once

#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "Base/DUtil.h"
#include "Net/DNet.h"
#include "Net/DSelectServer.h"
#include <locale>
#include <string>

class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter , public DTCPServerSink, public DTCPDataSink
{
public:
    enum { IDD = IDD_MAINDIALOG };

    CMainDlg()
    {

    }

    virtual DVoid OnListening(DTCPServer* sock, DUInt16 wPort)
    {

    }

    virtual DVoid OnListenOK(DTCPServer* sock, DUInt16 wPort)
    {

    }

    virtual DVoid OnListenError(DTCPServer* sock, DUInt32 code, std::string strReason)
    {

    }

    virtual DVoid OnListenStop(DTCPServer* sock, DUInt32 code, std::string strReason)
    {

    }

    virtual DVoid OnNewConn(DTCPServer* sock, DTCPClient* newsock)
    {

    }


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

        DNet::Init();

        return TRUE;
    }

    LRESULT OnLog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        CString strLog;
        /*if (wParam == DEVENT_SERVER_READY)
        {
            strLog.Format(L"Server is listened at %hd\r\n", (DUInt16)lParam);
        }
        else if (wParam == DEVENT_SERVER_NEWCONN)
        {
            strLog.Format(L"New Connection from port: %d\r\n", (DUInt32)lParam);
        }*/
        AppendLog((wchar_t*)strLog.GetString());
        return 0;
    }

    LRESULT OnStartServer(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString strPort;
        m_port.GetWindowText(strPort);
        DUInt16 port = _wtoi(strPort);

        m_server.SetListenSink(this);
        m_server.SetDataSink(this);
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

    DSelectServer m_server;
};
