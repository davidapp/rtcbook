#pragma once

#include "atl.h"
#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "Base/DBuffer.h"
#include "Base/DFile.h"
#include "Base/DUtil.h"
#include "atldlgs.h"

wchar_t err_reason[5][15] = {
    L"OK",
    L"Zero Length",
    L"Invalid Char",
    L"Invalid Length",
    L""
};

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
        COMMAND_RANGE_HANDLER(IDOK, IDNO, OnCloseCmd)
        COMMAND_ID_HANDLER(IDC_ISHEX, OnClickISHex)
        COMMAND_ID_HANDLER(IDC_ISBASE64, OnClickIsBase64)
        COMMAND_ID_HANDLER(IDC_TOLINE, OnClickToLine)
        COMMAND_ID_HANDLER(IDC_TOLIST, OnClickToList)
        COMMAND_ID_HANDLER(IDC_TOBASE64, OnClickToBase64)
        COMMAND_ID_HANDLER(IDC_FROMBASE64, OnClickFromBase64)
        COMMAND_ID_HANDLER(IDC_OPENFILE, OnClickOpenFile)
        COMMAND_ID_HANDLER(IDC_RANDOM100, OnClickRandom100)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());

        m_hex = GetDlgItem(IDC_HEXEDIT);
        m_base = GetDlgItem(IDC_BASEEDIT);

        DBuffer buf(100);
        buf.FillWithRandom();
        std::string hexList = buf.ToHexList();
        m_hex.SetWindowText(DUtil::s2ws(hexList).c_str());
        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        ::EndDialog(m_hWnd, wID);
        m_hWnd = NULL;
        ::PostQuitMessage(0);
        return 0;
    }

    LRESULT OnClickISHex(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_hex.GetWindowText(str);
        std::string strA = DUtil::ws2s(std::wstring(str));
        DUInt32 reason = 0;
        DBool bOK = DBuffer::IsValidHexStr(strA.c_str(), &reason);
        if (!bOK)
        {
            CString err;
            err.Format(L"Not HEX, reason=%s", err_reason[reason]);
            MessageBox(err, L"Tips", MB_ICONERROR);
            return 0;
        }
        DBuffer buf;
        buf.InitWithHexString(strA);
        DUInt32 uSize = buf.GetSize();
        CString msg;
        msg.Format(L"It's HEX, %d bytes", uSize);
        MessageBox(msg, L"Tips", MB_OK);
        return 0;
    }

    LRESULT OnClickIsBase64(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_base.GetWindowText(str);
        std::string strA = DUtil::ws2s(std::wstring(str));
        DUInt32 reason = 0;
        DBool bOK = DBuffer::IsValidBase64Str(strA.c_str(), &reason);
        if (!bOK)
        {
            CString err;
            err.Format(L"Not Base64, reason=%s", err_reason[reason]);
            MessageBox(err, L"Tips", MB_ICONERROR);
            return 0;
        }
        DUInt32 uSize = strA.length();
        CString msg;
        msg.Format(L"It's Base64, %d chars, %d bytes", uSize, DBuffer::GetBase64BufSize(strA.c_str()));
        MessageBox(msg, L"Tips", MB_OK);
        return 0;
    }

    LRESULT OnClickToLine(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_hex.GetWindowText(str);
        std::string strA = DUtil::ws2s(std::wstring(str));
        DUInt32 reason = 0;
        DBool bOK = DBuffer::IsValidHexStr(strA.c_str(), &reason);
        if (!bOK)
        {
            CString err;
            err.Format(L"Not HEX, reason=%s", err_reason[reason]);
            MessageBox(err, L"Tips", MB_ICONERROR);
            return 0;
        }
        DBuffer buf;
        buf.InitWithHexString(strA);
        std::string strHA = buf.ToHexString();
        m_hex.SetWindowText(DUtil::s2ws(strHA).c_str());
        return 0;
    }

    LRESULT OnClickToList(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_hex.GetWindowText(str);
        std::string strA = DUtil::ws2s(std::wstring(str));
        DUInt32 reason = 0;
        DBool bOK = DBuffer::IsValidHexStr(strA.c_str(), &reason);
        if (!bOK)
        {
            CString err;
            err.Format(L"Not HEX, reason=%s", err_reason[reason]);
            MessageBox(err, L"Tips", MB_ICONERROR);
            return 0;
        }
        DBuffer buf;
        buf.InitWithHexString(strA);
        std::string strHA = buf.ToHexList();
        m_hex.SetWindowText(DUtil::s2ws(strHA).c_str());
        return 0;
    }

    LRESULT OnClickToBase64(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_hex.GetWindowText(str);
        std::string strA = DUtil::ws2s(std::wstring(str));
        DUInt32 reason = 0;
        DBool bOK = DBuffer::IsValidHexStr(strA.c_str(), &reason);
        if (!bOK)
        {
            CString err;
            err.Format(L"Not HEX, reason=%s", err_reason[reason]);
            MessageBox(err, L"Tips", MB_ICONERROR);
            return 0;
        }
        DBuffer buf;
        buf.InitWithHexString(strA);
        std::string strBA = buf.ToBase64String();
        m_base.SetWindowText(DUtil::s2ws(strBA).c_str());
        return 0;
    }

    LRESULT OnClickFromBase64(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_base.GetWindowText(str);
        DBuffer buf;
        std::string strA = DUtil::ws2s(std::wstring(str));
        buf.InitWithBase64String(strA);
        std::string strAList = buf.ToHexList();
        m_hex.SetWindowText(DUtil::s2ws(strAList).c_str());
        return 0;
    }

    LRESULT OnClickOpenFile(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"All Files\0*.*\0", m_hWnd);
        if (dlg.DoModal() == IDOK)
        {
            DFile file;
            file.OpenFileRead(DUtil::ws2s(dlg.m_szFileName).c_str());
            // Max 100KB 
            DUInt32 readsize = 100 * 1024;
            if (file.GetSize() < readsize) readsize = (DUInt32)file.GetSize();
            DBuffer buf = file.Read(readsize);
            std::string hexList = buf.ToHexList();
            m_hex.SetWindowText(DUtil::s2ws(hexList).c_str());
        }
        return 0;
    }

    LRESULT OnClickRandom100(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DBuffer buf(100);
        buf.FillWithRandom();
        std::string hexList = buf.ToHexList();
        m_hex.SetWindowText(DUtil::s2ws(hexList).c_str());
        return 0;
    }


    CEdit	m_hex;
    CEdit	m_base;

    CButton m_tobase64;
    CButton m_frombase64;
    CButton m_ishex;
    CButton m_toline;
    CButton m_tolist;
    CButton m_isbase64;
};
