#pragma once

#include "atl.h"
#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "Base/DBuffer.h"
#include "Base/DUtil.h"
#include "Base/DUTF8.h"

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
        COMMAND_ID_HANDLER(IDC_CHARTOALL, OnClickT2All)
        COMMAND_ID_HANDLER(IDC_ANSI, OnClickA2T)
        COMMAND_ID_HANDLER(IDC_UNICODE, OnClickU2T)
        COMMAND_ID_HANDLER(IDC_ISUNICODE, OnClickIsU)
        COMMAND_ID_HANDLER(IDC_UTF8, OnClickU82T)
        COMMAND_ID_HANDLER(IDC_ISUTF8, OnClickIsU8)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());

        m_text = GetDlgItem(IDC_TEXT);
        m_ansibin = GetDlgItem(IDC_ANSIBIN);
        m_unicodebin = GetDlgItem(IDC_UNICODEBIN);
        m_utf8bin = GetDlgItem(IDC_UTF8BIN);

        m_text.SetWindowText(L"¹þ¹þÄãºÃ123");
        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        ::EndDialog(m_hWnd, wID);
        m_hWnd = NULL;
        PostQuitMessage(0);
        return 0;
    }

    LRESULT OnClickT2All(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_text.GetWindowText(str);

        std::string strA = DUtil::ws2s(str.GetString()).c_str();
        DBuffer bufStrA((DByte*)strA.c_str(), strA.length());
        std::string bufStrAHex = bufStrA.ToHexList(8);
        m_ansibin.SetWindowText(DUtil::s2ws(bufStrAHex.c_str()).c_str());

        DBuffer bufStr((DByte*)str.GetString(), str.GetLength()*2);
        std::string bufStrHex = bufStr.ToHexList(8);
        m_unicodebin.SetWindowText(DUtil::s2ws(bufStrHex.c_str()).c_str());

        std::string strU8A = DUTF8::UCS2ToUTF8((DUInt16*)str.GetString(), str.GetLength()*2);
        DBuffer bufU8A((DByte*)strU8A.c_str(), strU8A.length());
        std::string strU8AHex = bufU8A.ToHexList(8);
        m_utf8bin.SetWindowText(DUtil::s2ws(strU8AHex.c_str()).c_str());

        return 0;
    }

    LRESULT OnClickA2T(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_ansibin.GetWindowText(str);
        DBuffer bufText;
        bufText.InitWithHexString(DUtil::ws2s(str.GetString()).c_str());
        std::string strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
        std::wstring strShow = DUtil::s2ws(strAnsi);
        m_text.SetWindowTextW(strShow.c_str());
        return 0;
    }

    LRESULT OnClickU2T(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_unicodebin.GetWindowText(str);
        DBuffer bufText;
        bufText.InitWithHexString(DUtil::ws2s(str.GetString()).c_str());
        std::wstring strShow((const DWChar*)bufText.GetBuf(), bufText.GetSize()/2);
        m_text.SetWindowTextW(strShow.c_str());
        return 0;
    }

    LRESULT OnClickIsU(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_unicodebin.GetWindowText(str);
        DBuffer bufText;
        bufText.InitWithHexString(DUtil::ws2s(str.GetString()).c_str());
        std::string strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
        if (::IsTextUnicode(strAnsi.c_str(), strAnsi.size(), NULL))
        {
            MessageBox(L"Yes", L"IsTextUnicode");
        }
        else
        {
            MessageBox(L"No", L"IsTextUnicode");
        }
        return 0;
    }

    LRESULT OnClickU82T(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_utf8bin.GetWindowText(str);
        DBuffer bufText;
        bufText.InitWithHexString(DUtil::ws2s(str.GetString()).c_str());
        std::string strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
        std::wstring strShow = DUTF8::UTF8ToUCS2((DByte*)strAnsi.c_str(), strAnsi.size());
        m_text.SetWindowTextW(strShow.c_str());
        return 0;
    }

    LRESULT OnClickIsU8(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_utf8bin.GetWindowText(str);
        DBuffer bufText;
        bufText.InitWithHexString(DUtil::ws2s(str.GetString()).c_str());
        std::string strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
        if (DUTF8::isUTF8((DByte*)strAnsi.c_str(), strAnsi.length()))
        {
            MessageBox(L"Yes", L"IsTextUTF8");
        }
        else
        {
            MessageBox(L"No", L"IsTextUTF8");
        }
        return 0;
    }

    CEdit m_text;
    CEdit m_ansibin;
    CEdit m_unicodebin;
    CEdit m_utf8bin;

    CButton m_t2all;
    CButton m_a2t;
    CButton m_u2t;
    CButton m_u82t;
    CButton m_isu;
    CButton m_isu8;
};
