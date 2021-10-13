//
//  MainDlg.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 07/20/2019 for Dream.
//  Copyright 2019. All rights reserved.
//

#pragma once

#include "WinExtra/DWinUI/DDlg.h"
#include "WinExtra/DWinUI/DUser.h"
#include "WinExtra/DWinUI/DCtrl.h"
#include "resource.h"
#include "Base/DString.h"
#include "Base/DBuffer.h"

wchar_t err_reason[5][15] = {
	L"OK",
	L"Zero Length",
	L"Invalid Char",
	L"Invalid Length",
	L""
};

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
		COMMAND_ID_HANDLER(IDC_ISHEX, OnClickISHex)
		COMMAND_ID_HANDLER(IDC_ISBASE64, OnClickIsBase64)
		COMMAND_ID_HANDLER(IDC_ISBASE58, OnClickIsBase58)
		COMMAND_ID_HANDLER(IDC_TOLINE, OnClickToLine)
		COMMAND_ID_HANDLER(IDC_TOLIST, OnClickToList)
		COMMAND_ID_HANDLER(IDC_TOBASE64, OnClickToBase64)
		COMMAND_ID_HANDLER(IDC_TOBASE58, OnClickToBase58)
		COMMAND_ID_HANDLER(IDC_FROMBASE64, OnClickFromBase64)
		COMMAND_ID_HANDLER(IDC_FROMBASE58, OnClickFromBase58)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ExecuteDlgInit(t_wDlgTemplateID);
		if (t_bCenter)
			CenterWindow(GetParent());

		m_hex = GetDlgItem(IDC_HEXEDIT);
		m_base = GetDlgItem(IDC_BASEEDIT);

		DBuffer buf(100);
		buf.FillWithRandom();
		DStringA hexList = buf.ToHexList();
		m_hex.SetWindowText(hexList.ToUnicode());

		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		::EndDialog(m_hWnd, wID);
		PostQuitMessage(0);
		return 0;
	}

	LRESULT OnClickISHex(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_hex.GetWindowText();
		DStringA strA = str.ToAnsi();
		DUInt32 reason = 0;
		DBool bOK = DBuffer::IsValidHexStr(strA.GetStr(), &reason);
		if (!bOK)
		{
			DString err;
			err.Format(L"Not HEX, reason=%S", err_reason[reason]);
			DMessageBox(m_hWnd, err.GetStr(), L"Tips");
			return 0;
		}
		DBuffer buf;
		buf.InitWithHexString(strA);
		DUInt32 uSize = buf.GetSize();
		DString msg;
		msg.Format(L"It's HEX, %d bytes", uSize);
		DMessageBox(m_hWnd, msg.GetStr(), L"Tips");
		return 0;
	}

	LRESULT OnClickIsBase58(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{

		return 0;
	}

	LRESULT OnClickIsBase64(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_base.GetWindowText();
		DStringA strA = str.ToAnsi();
		DUInt32 reason = 0;
		DBool bOK = DBuffer::IsValidBase64Str(strA.GetStr(), &reason);
		if (!bOK)
		{
			DString err;
			err.Format(L"Not Base64, reason=%S", err_reason[reason]);
			DMessageBox(m_hWnd, err.GetStr(), L"Tips");
			return 0;
		}
		DUInt32 uSize = strA.GetDataLength();
		DString msg;
		msg.Format(L"It's Base64, %d chars, %d bytes", uSize, DBuffer::GetBase64BufSize(strA.GetStr()));
		DMessageBox(m_hWnd, msg.GetStr(), L"Tips");
		return 0;
	}

	LRESULT OnClickToLine(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_hex.GetWindowText();
		DStringA strA = str.ToAnsi();
		DUInt32 reason = 0;
		DBool bOK = DBuffer::IsValidHexStr(strA.GetStr(), &reason);
		if (!bOK)
		{
			DString err;
			err.Format(L"Not HEX, reason=%S", err_reason[reason]);
			DMessageBox(m_hWnd, err.GetStr(), L"Tips");
			return 0;
		}
		DBuffer buf;
		buf.InitWithHexString(strA);
		DStringA strHA = buf.ToHexString();
		m_hex.SetWindowText(strHA.ToUnicode());
		return 0;
	}

	LRESULT OnClickToList(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_hex.GetWindowText();
		DStringA strA = str.ToAnsi();
		DUInt32 reason = 0;
		DBool bOK = DBuffer::IsValidHexStr(strA.GetStr(), &reason);
		if (!bOK)
		{
			DString err;
			err.Format(L"Not HEX, reason=%S", err_reason[reason]);
			DMessageBox(m_hWnd, err.GetStr(), L"Tips");
			return 0;
		}
		DBuffer buf;
		buf.InitWithHexString(strA);
		DStringA strHA = buf.ToHexList();
		m_hex.SetWindowText(strHA.ToUnicode());
		return 0;
	}

	LRESULT OnClickToBase64(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_hex.GetWindowText();
		DStringA strA = str.ToAnsi();
		DUInt32 reason = 0;
		DBool bOK = DBuffer::IsValidHexStr(strA.GetStr(), &reason);
		if (!bOK)
		{
			DString err;
			err.Format(L"Not HEX, reason=%S", err_reason[reason]);
			DMessageBox(m_hWnd, err.GetStr(), L"Tips");
			return 0;
		}
		DBuffer buf;
		buf.InitWithHexString(strA);
		DStringA strBA = buf.ToBase64String();
		m_base.SetWindowText(strBA.ToUnicode());
		return 0;
	}

	LRESULT OnClickToBase58(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_hex.GetWindowText();
		DStringA strA = str.ToAnsi();
		DUInt32 reason = 0;
		/*DBool bOK = DBuffer::IsValidHexStr(strA.GetStr(), &reason);
		if (!bOK)
		{
			DString err;
			err.Format(L"Not HEX, reason=%S", err_reason[reason]);
			DMessageBox(m_hWnd, err.GetStr(), L"Tips");
			return 0;
		}*/
		
		DBuffer buf;
		buf.InitWithHexString(strA);
		DStringA strBA = buf.ToBase58String();
		m_base.SetWindowText(strBA.ToUnicode());

		//DBuffer::TestBase58();
		return 0;
	}

	LRESULT OnClickFromBase64(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DString str = m_base.GetWindowText();
		DBuffer buf;
		buf.InitWithBase64String(str.ToAnsi());
		DStringA strA = buf.ToHexList();
		m_hex.SetWindowText(strA.ToUnicode());
		return 0;
	}

	LRESULT OnClickFromBase58(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{

		return 0;
	}

	DEdit	m_hex;
	DEdit	m_base;

	DButton m_tobase64;
	DButton m_frombase64;
	DButton m_tobase58;
	DButton m_frombase58;
	DButton m_ishex;
	DButton m_toline;
	DButton m_tolist;
	DButton m_isbase64;
	DButton m_isbase58;
};
