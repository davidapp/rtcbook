#pragma once

#include "atl.h"
#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "Base/DBuffer.h"
#include "Base/DUtil.h"
#include "atldlgs.h"

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
        PostQuitMessage(0);
        return 0;
    }

    LRESULT OnClickT2All(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_text.GetWindowText(str);

        std::string strA = DUtil::ws2s(str.GetString()).c_str();
        /*DBuffer bufStrA = strA.ToBuffer();
        DStringA bufStrAHex = bufStrA.ToHexList(8);
        m_ansibin.SetWindowText(bufStrAHex.ToUnicode());

        DBuffer bufStr = str.ToBuffer();
        DStringA bufStrHex = bufStr.ToHexList(8);
        m_unicodebin.SetWindowText(bufStrHex.ToUnicode());

        DStringA strU8A = str.ToUTF8();
        DBuffer bufU8A = strU8A.ToBuffer();
        DStringA strU8AHex = bufU8A.ToHexList(8);
        m_utf8bin.SetWindowText(strU8AHex.ToUnicode());
        */
        return 0;
    }

    LRESULT OnClickA2T(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_ansibin.GetWindowText(str
        );
        DBuffer bufText;
        bufText.InitWithHexString(str.ToAnsi());
        DStringA strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
        DString strShow = strAnsi.ToUnicode();
        m_text.SetWindowTextW(strShow);
        return 0;
    }

    LRESULT OnClickU2T(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        DString str = m_unicodebin.GetWindowText();
        DBuffer bufText;
        bufText.InitWithHexString(str.ToAnsi());
        DString strShow((const DWChar*)bufText.GetBuf(), bufText.GetSize());
        m_text.SetWindowTextW(strShow);
        return 0;
    }

    LRESULT OnClickIsU(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_unicodebin.GetWindowText(str);
        DBuffer bufText;
        bufText.InitWithHexString(DUtil::ws2s(str.GetString()).c_str());
        std::string strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
        /*if (strAnsi.IsTextUnicode())
        {
            MessageBox(L"Yes", L"IsTextUnicode");
        }
        else
        {
            MessageBox(L"No", L"IsTextUnicode");
        }*/
        return 0;
    }

    LRESULT OnClickU82T(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        /*DString str = m_utf8bin.GetWindowText();
        DBuffer bufText;
        bufText.InitWithHexString(str.ToAnsi());
        DStringA strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
        DString strShow = strAnsi.UTF8ToUnicode();
        m_text.SetWindowTextW(strShow);*/
        return 0;
    }

    LRESULT OnClickIsU8(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString str;
        m_utf8bin.GetWindowText(str);
        DBuffer bufText;
        bufText.InitWithHexString(DUtil::ws2s(str.GetString()).c_str());
        std::string strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
        /*if (strAnsi.IsTextUTF8())
        {
            MessageBox(L"Yes", L"IsTextUTF8");
        }
        else
        {
            MessageBox(L"No", L"IsTextUTF8");
        }*/
        return 0;
    }

    //DStatic m_title;
    CEdit	m_text;
    CEdit	m_ansibin;
    CEdit	m_unicodebin;
    CEdit	m_utf8bin;
    CButton m_t2all;
    CButton m_a2t;
    CButton m_u2t;
    CButton m_u82t;
    CButton m_isu;
    CButton m_isu8;
};
