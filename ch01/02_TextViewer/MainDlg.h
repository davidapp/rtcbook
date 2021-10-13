//
//  DMainDlg.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 07/20/2019 for Dream.
//  Copyright 2019. All rights reserved.
//

#pragma once

#include "DConfig.h"
#include "DTypes.h"
#include "WinExtra/DWinUI/DDlg.h"
#include "WinExtra/DWinUI/DUser.h"
#include "WinExtra/DWinUI/DCtrl.h"
#include "resource.h"
#include "Base/DString.h"
#include "Base/DBuffer.h"

template <WORD t_wDlgTemplateID, BOOL t_bCenter/* = TRUE*/>
class DMainDialog : public DDlgImplBase
{
public:
	typedef DMainDialog<t_wDlgTemplateID, t_bCenter> thisClass;

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		DAssert(m_hWnd == NULL);
		DWinApp::AddCreateWndData(&m_thunk.cd, (DDlgImplBase*)this);
		INT_PTR nRet = ::DialogBox(DWinApp::GetResInstance(), MAKEINTRESOURCE(t_wDlgTemplateID), hWndParent, StartDialogProc);
		m_hWnd = NULL;
		return nRet;
	}

	BEGIN_MSG_MAP(thisClass)
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
		ExecuteDlgInit(t_wDlgTemplateID);
		if (t_bCenter)
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
		DString str = m_text.GetWindowText();
		
		DStringA strA = str.ToAnsi();
		DBuffer bufStrA = strA.ToBuffer();
		DStringA bufStrAHex = bufStrA.ToHexList(8);
		m_ansibin.SetWindowText(bufStrAHex.ToUnicode());

		DBuffer bufStr = str.ToBuffer();
		DStringA bufStrHex = bufStr.ToHexList(8);
		m_unicodebin.SetWindowText(bufStrHex.ToUnicode());

		DStringA strU8A = str.ToUTF8();
		DBuffer bufU8A = strU8A.ToBuffer();
		DStringA strU8AHex = bufU8A.ToHexList(8);
		m_utf8bin.SetWindowText(strU8AHex.ToUnicode());

		return 0;
	}

	LRESULT OnClickA2T(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_ansibin.GetWindowText();
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
		DString str = m_unicodebin.GetWindowText();
		DBuffer bufText;
		bufText.InitWithHexString(str.ToAnsi());
		DStringA strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
		if (strAnsi.IsTextUnicode())
		{
			DMessageBox(NULL, L"Yes", L"IsTextUnicode");
		}
		else
		{
			DMessageBox(NULL, L"No", L"IsTextUnicode");
		}
		return 0;
	}

	LRESULT OnClickU82T(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_utf8bin.GetWindowText();
		DBuffer bufText;
		bufText.InitWithHexString(str.ToAnsi());
		DStringA strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
		DString strShow = strAnsi.UTF8ToUnicode();
		m_text.SetWindowTextW(strShow);
		return 0;
	}

	LRESULT OnClickIsU8(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_utf8bin.GetWindowText();
		DBuffer bufText;
		bufText.InitWithHexString(str.ToAnsi());
		DStringA strAnsi((const DChar*)bufText.GetBuf(), bufText.GetSize());
		if (strAnsi.IsTextUTF8())
		{
			DMessageBox(NULL, L"Yes", L"IsTextUTF8");
		}
		else
		{
			DMessageBox(NULL, L"No", L"IsTextUTF8");
		}
		return 0;
	}

	//DStatic m_title;
	DEdit	m_text;
	DEdit	m_ansibin;
	DEdit	m_unicodebin;
	DEdit	m_utf8bin;
	DButton m_t2all;
	DButton m_a2t;
	DButton m_u2t;
	DButton m_u82t;
	DButton m_isu;
	DButton m_isu8;
};
