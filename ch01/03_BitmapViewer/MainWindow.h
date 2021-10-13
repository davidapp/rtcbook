//
//  DMainWindow.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 02/18/2018 for Dream.
//  Copyright 2018. All rights reserved.
//

#pragma once

#include <math.h>
#include "resource.h"
#include "Base/DMisc.h"
#include "Misc/DBitDump.h"
#include "File/Image/DBmpFile.h"
#include "File/Image/DJpegFile.h"
#include "Codec/DQuickTable.h"
#include "WinExtra/DWinApp.h"
#include "WinExtra/Tools/DConsole.h"
#include "WinExtra/Tools/DWinDbg.h"
#include "WinExtra/DWinUI/DWnd.h"
#include "WinExtra/DWinUI/DCrack.h"
#include "WinExtra/DWinUI/DGDI.h"
#include "WinExtra/DWinUI/DCtrl.h"
#include "WinExtra/DWinUI/DCommDlg.h"
#include "WinExtra/DreamUI/DRender.h"

#include "DKeybdWindow.h"
#include "DMouseWindow.h"
#include "DContainWindow.h"
#include "DSplitWindow.h"
#include "DScrollWindow.h"
#include "DFrameWindow.h"

#define DRAW_TYPE_NONE 0
#define DRAW_TYPE_GRID 1
#define DRAW_TYPE_POLYGON 2
#define DRAW_TYPE_SINE 3
#define DRAW_TYPE_RECT 4
#define DRAW_TYPE_ELLIPSE 5
#define DRAW_TYPE_ROUNDRECT 6
#define DRAW_TYPE_ARC 7
#define DRAW_TYPE_CHORD 8
#define DRAW_TYPE_PIE 9
#define DRAW_TYPE_DEMO 10
#define DRAW_TYPE_BEZIER 11
#define DRAW_TYPE_PEN 12

#define DRAW_TYPE_BRUSH 13
#define DRAW_TYPE_PENTAGRAM 14
#define DRAW_TYPE_WINDING 15
#define DRAW_TYPE_MAPMODE 16
#define DRAW_TYPE_CLOVER 17

#define NUM    1000
#define TWOPI  (2 * 3.14159)

class DIdleDraw : public DIdleHandler
{
public:
	DIdleDraw(HWND hWnd)
	{
		m_hWnd = hWnd;
	}
	BOOL OnIdle()
	{
		HBRUSH hBrush;
		HDC    hdc;
		RECT   rect;
		int cxClient = 100;
		int cyClient = 100;

		SetRect(&rect, 300, 300, 300+DMisc::Random32()%cxClient, 300+DMisc::Random32()%cyClient);
		hBrush = CreateSolidBrush(RGB(DMisc::RandomChar(), DMisc::RandomChar(), DMisc::RandomChar()));
		hdc = GetDC(m_hWnd);

		FillRect(hdc, &rect, hBrush);
		ReleaseDC(m_hWnd, hdc);
		DeleteObject (hBrush);
		return TRUE;
	}
public:
	HWND m_hWnd;
};

class DMainWindow : public DWndImpl<DMainWindow>
{
public:
	DECLARE_DWND_CLASS_EX(L"MainWindow", D_CS_NORMALFRAME, COLOR_WINDOW)	//if bkgnd==0 we must paint in erase bkgnd

public:
	DMainWindow()
	{
		DWndClassInfo& wci = GetWndClassInfo();
		if (!wci.m_atom)
		{
			wci.m_wc.hIcon = LoadIcon(DWinApp::GetResInstance(), MAKEINTRESOURCE(IDI_ICON));
			wci.m_wc.hIconSm = LoadIcon(DWinApp::GetResInstance(), MAKEINTRESOURCE(IDI_ICON));
		}

		m_erased = 0;
		m_Height = 0;
		m_Width = 0;
		m_drawType = 0;
		m_drawText = 0;
		m_dwCharSet = 0;
		m_cxChar = 0;
		m_cyChar = 0;
		hRgnClip = NULL;
		DMemZero(hRgnTemp, 6 * sizeof(HRGN));
		DMemZero(bezier_apt, 4 * sizeof(POINT));
		DMemZero(wind_apt, 10 * sizeof(POINT));
	}

	BEGIN_MSG_MAP(DMainWindow)
		//Create
		MSG_WM_CREATE(OnCreate)

		//Size
		MSG_WM_SIZE(OnSize)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_HSCROLL(OnHScroll)

		//Paint
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_NCPAINT(OnNcPaint)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)

		//Focus
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_KILLFOCUS(OnKillFocus)

		//Mouse
		MSG_WM_MOUSEMOVE(OnMouseProcess)
		MSG_WM_LBUTTONDOWN(OnMouseProcess)
		MSG_WM_RBUTTONDOWN(OnMouseProcess)

		//Keyboard
		MSG_WM_CHAR(OnChar)
		MSG_WM_INPUTLANGCHANGE(OnInputLangChange)
		
		//Menus
		//System
		COMMAND_ID_HANDLER(ID_DC_RESET, OnReset)
		COMMAND_ID_HANDLER(ID_TEST_IN_FALSE, OnInFalse)
		COMMAND_ID_HANDLER(ID_TEST_IN_TRUE, OnInTrue)
		COMMAND_ID_HANDLER(ID_DC_DUMPSYSMETRIC, DumpSysMetric)
		COMMAND_ID_HANDLER(ID_TEST_DUMPDCCAPS, DumpDCCaps)
		COMMAND_ID_HANDLER(ID_TEST_DUMPDCATTR, DumpDCAttr)
		COMMAND_ID_HANDLER(ID_WINDOW_STYLE, OnDumpStyle)
		COMMAND_ID_HANDLER(ID_SYSTEM_EXIT, OnExit)
		
		//Line
		COMMAND_ID_HANDLER(ID_LINE_GRID, OnDrawGrid)
		COMMAND_ID_HANDLER(ID_LINE_POLYGON, OnDrawPolygon)
		COMMAND_ID_HANDLER(ID_LINE_SINE, OnDrawSine)
		COMMAND_ID_HANDLER(ID_LINE_RECT, OnDrawRect)
		COMMAND_ID_HANDLER(ID_LINE_ELLIPSE, OnDrawEllipse)
		COMMAND_ID_HANDLER(ID_LINE_ROUNDRECT, OnDrawRoundRect)
		COMMAND_ID_HANDLER(ID_LINE_ARC, OnDrawArc)
		COMMAND_ID_HANDLER(ID_LINE_CHOND, OnDrawChord)
		COMMAND_ID_HANDLER(ID_LINE_PIE, OnDrawPie)
		COMMAND_ID_HANDLER(ID_LINE_DEMO, OnDrawDemo)
		COMMAND_ID_HANDLER(ID_LINE_BEZIERSPLINE, OnDrawBezier)
		COMMAND_ID_HANDLER(ID_LINE_PEN, OnDrawPen)
		
		//Rect
		COMMAND_ID_HANDLER(ID_RECT_BRUSH, OnDrawBrush)
		COMMAND_ID_HANDLER(ID_RECT_PENTAGRAM, OnDrawPentaGram)
		COMMAND_ID_HANDLER(ID_RECT_WINDING, OnDrawWinding)
		COMMAND_ID_HANDLER(ID_RECT_MAPMODE, OnDrawMapMode)
		COMMAND_ID_HANDLER(ID_RECT_CLOVER, OnDrawClover)
		COMMAND_RANGE_HANDLER(ID_MMTEXT, ID_ANISOTROPIC, OnSetMapMode)

		//Text
		COMMAND_ID_HANDLER(ID_DC_DUMPTEXTMETRIC, DumpTextMetrics)
		COMMAND_ID_HANDLER(ID_DC_TEXTOUT, OnTextOut)
		COMMAND_ID_HANDLER(ID_DC_PRINTINPUT, OnPrintInput)

		//Bitmap
		COMMAND_ID_HANDLER(ID_BITMAP_LOADANDDRAW, OnLoadAndDraw)
		COMMAND_ID_HANDLER(ID_LOAD_JPEG, OnLoadJpeg)

		//Window
		COMMAND_ID_HANDLER(ID_WINDOW_KEYBOARDVIEW, OnOpenKeyboard)
		COMMAND_ID_HANDLER(ID_WINDOW_MOUSEVIEW, OnOpenMouse)
		COMMAND_ID_HANDLER(ID_WINDOW_CONSOLE, OnOutputConsole)

		//DWinApp
		COMMAND_ID_HANDLER(ID_DWINAPP_WNDCLASS, OnWndClassTest)
		
		//DWnd
		COMMAND_ID_HANDLER(ID_DWND_DWNDTEST, OnDWndTest)
		COMMAND_ID_HANDLER(ID_DWND_DCONTAINEDWND, OnDContainedWndTest)

		//CommDlg
		COMMAND_ID_HANDLER(ID_COMMDLG_OPENFILEDIALOG, OnOpenFileDialog)
		COMMAND_ID_HANDLER(ID_COMMDLG_SAVEFILEDIALOG, OnSaveFileDialog)
		COMMAND_ID_HANDLER(ID_COMMDLG_SIMPLEFILEDIALOG, OnSimpleDialog)
		COMMAND_ID_HANDLER(ID_COMMDLG_MULTIFILEDIALOG, OnMultiFileDialog)
		COMMAND_ID_HANDLER(ID_COMMDLG_SHELLOPENDIALOG, OnShellOpenDialog)
		COMMAND_ID_HANDLER(ID_COMMDLG_SHELLSAVEDIALOG, OnShellSaveDialog)
		COMMAND_ID_HANDLER(ID_COMMDLG_FOLDERDIALOG, OnFolderDialog)
		COMMAND_ID_HANDLER(ID_COMMDLG_FONTDIALOG, OnFontDialog)
		COMMAND_ID_HANDLER(ID_COMMDLG_RICHEDITFONTDIALOG, OnRichEditFontDialog)
		COMMAND_ID_HANDLER(ID_COMMDLG_COLORDIALOG, OnColorDialog)

		//DSplit
		COMMAND_ID_HANDLER(ID_DWINUI_DSPLIT, OnOpenSplitterWnd)

		//DScroll
		COMMAND_ID_HANDLER(ID_DWINUI_DSCROLL, OnOpenScrollWnd)
	

	END_MSG_MAP()

	//Create
	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		m_erased = 0;
		m_drawType = DRAW_TYPE_NONE;
		m_drawText = 0;
		m_dwCharSet = DEFAULT_CHARSET;
		m_font = D_DEFAULT_GUI_FONT;

		DClientDC dc(m_hWnd);
		dc.SelectFont(m_font);

		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		m_cxChar = tm.tmAveCharWidth;
		m_cyChar = tm.tmHeight;

		//DIdleDraw *pIdle = new DIdleDraw(m_hWnd);
		//DMessageLoop::GetCurrentLoop()->AddIdleHandler(pIdle);

		console.Create(L"Dream Debug Console");
		console.Show();
		return 0;
	}

	//Size
	void OnSize(UINT nType, DSize size)
	{
		m_Height = size.h;
		m_Width = size.w;

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin = 0;
		si.nMax = 1500;
		si.nPage = m_Height;
		SetScrollInfo(SB_VERT, &si, TRUE);

		SCROLLINFO sih;
		sih.cbSize = sizeof(SCROLLINFO);
		sih.fMask = SIF_PAGE | SIF_RANGE;
		sih.nMin = 0;
		sih.nMax = 1500;
		sih.nPage = m_Width;
		SetScrollInfo(SB_HORZ, &sih, TRUE);


		bezier_apt[0].x = m_Width/4;
		bezier_apt[0].y = m_Height/2;

		bezier_apt[1].x = m_Width/2;
		bezier_apt[1].y = m_Height/4;

		bezier_apt[2].x = m_Width/2;
		bezier_apt[2].y = 3*m_Height/4;

		bezier_apt[3].x = 3*m_Width/4;
		bezier_apt[3].y = m_Height/2;
		
		HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
		ShowCursor(TRUE);

		if (hRgnClip)
			DeleteObject (hRgnClip) ;

		hRgnTemp[0] = CreateEllipticRgn(0, m_Height / 3, m_Width / 2, 2 * m_Height / 3);
		hRgnTemp[1] = CreateEllipticRgn(m_Width / 2, m_Height / 3, m_Width, 2 * m_Height / 3);
		hRgnTemp[2] = CreateEllipticRgn(m_Width / 3, 0, 2 * m_Width / 3, m_Height / 2);
		hRgnTemp[3] = CreateEllipticRgn(m_Width / 3, m_Height / 2, 2 * m_Width / 3, m_Height);
		hRgnTemp[4] = CreateRectRgn(0, 0, 1, 1);
		hRgnTemp[5] = CreateRectRgn(0, 0, 1, 1);
		hRgnClip    = CreateRectRgn(0, 0, 1, 1);

		CombineRgn(hRgnTemp[4], hRgnTemp[0], hRgnTemp[1], RGN_OR);
		CombineRgn(hRgnTemp[5], hRgnTemp[2], hRgnTemp[3], RGN_OR);
		CombineRgn(hRgnClip,    hRgnTemp[4], hRgnTemp[5], RGN_XOR);

		for (int i = 0 ; i < 6 ; i++)
			DeleteObject(hRgnTemp[i]);

		SetCursor(hCursor);
		ShowCursor(FALSE);
	}

	DVoid OnVScroll(UINT nSBCode, UINT nPos, DScrollBar pScrollBar)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(SB_VERT, &si);
		int iVertPos = si.nPos;

		switch (nSBCode)
		{
		case SB_TOP:
			si.nPos = si.nMin;	//SendMessage(m_hWnd, WM_VSCROLL, SB_TOP, 0);
			break;
		case SB_BOTTOM:
			si.nPos = si.nMax;	//SendMessage(m_hWnd, WM_VSCROLL, SB_BOTTOM, 0);
			break;
		case SB_LINEUP:
			si.nPos -= 1;
			break;
		case SB_LINEDOWN:
			si.nPos += 1;
			break;
		case SB_PAGEUP:
			si.nPos -= si.nPage / 2;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage / 2;
			break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(SB_VERT, &si, TRUE);
		GetScrollInfo(SB_VERT, &si);
		if (si.nPos != iVertPos)
		{
			ScrollWindow(0, iVertPos - si.nPos, NULL, NULL);	//Will generate WM_PAINT
			UpdateWindow();		//Will also sent a WM_PAINT
		}
	}

	DVoid OnHScroll(UINT nSBCode, UINT nPos, DScrollBar pScrollBar)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(SB_HORZ, &si);
		int iHorzPos = si.nPos;

		switch (nSBCode)
		{
		case SB_LINELEFT:
			si.nPos -= 1;
			break;
		case SB_LINERIGHT:
			si.nPos += 1;
			break;
		case SB_PAGELEFT:
			si.nPos -= si.nPage / 2;
			break;
		case SB_PAGERIGHT:
			si.nPos += si.nPage / 2;
			break;
		case SB_THUMBPOSITION:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(SB_HORZ, &si, TRUE);
		GetScrollInfo(SB_HORZ, &si);
		if (si.nPos != iHorzPos)
		{
			ScrollWindow(iHorzPos - si.nPos, 0, NULL, NULL);
			UpdateWindow();
		}
	}

	//Paint
	BOOL OnEraseBkgnd(DDCHandle dc)
	{
		if (m_erased)
		{
			RECT r;
			GetClientRect(&r);
			dc.FillRect(&r, (HBRUSH)::GetStockObject(WHITE_BRUSH));

			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;

			GetScrollInfo(SB_VERT, &si);
			int iVertPos = si.nPos;
			r.bottom = 100 - iVertPos;

			GetScrollInfo(SB_HORZ, &si);
			int iHorzPos = si.nPos;
			r.right -= iHorzPos;

			dc.FillRect(&r, (HBRUSH)::GetStockObject(GRAY_BRUSH));
		}
		//An application should return nonzero if it erases the background; otherwise, it should return zero. 
		return m_erased;
	}

	void OnNcPaint(DRgnHandle rgn)
	{
		//When wParam is 1, the entire window frame needs to be updated.
		/*if ((rgn == (HRGN)1) && (m_erased==1))
		{
			DWindowDC wdc(m_hWnd);
			wdc.MoveTo(0, 0, NULL);
			wdc.LineTo(100, 100);
			return;
		}*/
		SetMsgHandled(FALSE);
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DPaintDC dc(m_hWnd);
		RECT r;
		GetClientRect(&r);
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(SB_VERT, &si);
		int iVertPos = si.nPos;
		GetScrollInfo(SB_HORZ, &si);
		int iHorzPos = si.nPos;

		if (dc.m_ps.fErase)
		{
			//We erase bkgnd here
			dc.FillRect(&r, (HBRUSH)GetStockObject(WHITE_BRUSH));
			r.right -= iHorzPos;
			r.bottom = 100 - iVertPos;
			dc.FillRect(&r, (HBRUSH)::GetStockObject(GRAY_BRUSH));
		}

		//Draw
		if (m_drawText == 1)
		{
			dc.TextOut(0, 0, strPaintInfo, -1);
			m_drawText = 0;
		}
		else if (m_drawText == 2)
		{
			dc.TextOut(0, 0, m_Input, -1);
			m_drawText = 0;
			m_Input.Empty();
		}

		if (m_drawType == DRAW_TYPE_NONE)
		{
			dc.SelectFont(D_DEFAULT_GUI_FONT);
			GetClientRect(&r);
			r.top = (100 - iVertPos);
			r.bottom -= iVertPos;
			r.left = -iHorzPos;
			r.right -= iHorzPos;
			if (dc.m_ps.fErase)
			{
				strPaintInfo.Format(L"fErase=1 we should erase background in OnPaint, PaintRect=(%d,%d,%d,%d)", dc.m_ps.rcPaint.left, dc.m_ps.rcPaint.top, dc.m_ps.rcPaint.right, dc.m_ps.rcPaint.bottom);
				dc.DrawText(strPaintInfo, -1, &r, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
			else
			{
				strPaintInfo.Format(L"fErase=0 background has been already erased in OnEraseBkgnd, PaintRect=(%d,%d,%d,%d)", dc.m_ps.rcPaint.left, dc.m_ps.rcPaint.top, dc.m_ps.rcPaint.right, dc.m_ps.rcPaint.bottom);
				dc.DrawText(strPaintInfo, -1, &r, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
		}
		else if (m_drawType == DRAW_TYPE_GRID)
		{
			for (int i = 5; i <= 105; i += 10)
			{
				dc.MoveTo(i - iHorzPos, 105 - iVertPos, NULL);
				dc.LineTo(i - iHorzPos, 205 - iVertPos);
			}
			for (int j = 105; j <= 205; j += 10)
			{
				dc.MoveTo(5 - iHorzPos, j - iVertPos, NULL);
				dc.LineTo(105 - iHorzPos, j - iVertPos);
			}
		}
		else if (m_drawType == DRAW_TYPE_POLYGON)
		{
			POINT apt[5] = { 100, 100, 200, 100, 200, 200, 100, 200, 100, 100 };
			//Adjust by ScrollPos
			for (int i = 0; i < 5; i++)
			{
				apt[i].x -= iHorzPos;
				apt[i].y -= iVertPos;
			}

			dc.MoveTo(apt[0].x, apt[0].y, NULL);
			//for (int i=1; i<5; i++)	dc.LineTo(apt[i].x, apt[i].y);
			//The same as above line
			dc.PolylineTo(apt + 1, 4);

			//Polyline doesn't use or change the current position
			//dc.Polyline(apt, 4);
		}
		else if (m_drawType == DRAW_TYPE_SINE)
		{
			GetClientRect(&r);
			POINT apt[NUM];
			dc.MoveTo(-iHorzPos, r.bottom / 2 - iVertPos, NULL);
			dc.LineTo(r.right - iHorzPos, r.bottom / 2 - iVertPos);

			for (int i = 0; i < NUM; i++)
			{
				apt[i].x = (i * r.right / NUM) - iHorzPos;
				apt[i].y = ((int)(r.bottom / 2 * (1 - sin(TWOPI * i / NUM)))) - iVertPos;
			}
			dc.Polyline(apt, NUM);
		}
		else if (m_drawType == DRAW_TYPE_RECT)
		{
			//will have 12 point 10 around with 2 inside
			dc.Rectangle(1, 1, 5, 4);
		}
		else if (m_drawType == DRAW_TYPE_ELLIPSE)
		{
			dc.Ellipse(20, 20, 120, 80);
		}
		else if (m_drawType == DRAW_TYPE_ROUNDRECT)
		{
			//xCornerEllipse = (xRight - xLeft) / 4 ;
			//yCornerEllipse = (yBottom- yTop) / 4 ;
			dc.RoundRect(20, 20, 120, 80, 10, 10);
		}
		else if (m_drawType == DRAW_TYPE_ARC)
		{
			dc.Arc(20, 20, 120, 80, 70, 0, 10, 70);
		}
		else if (m_drawType == DRAW_TYPE_CHORD)
		{
			dc.Chord(20, 20, 120, 80, 70, 0, 10, 70);
		}
		else if (m_drawType == DRAW_TYPE_PIE)
		{
			dc.Pie(20, 20, 120, 80, 70, 0, 10, 70);
		}
		else if (m_drawType == DRAW_TYPE_DEMO)
		{
			dc.Rectangle(m_Width / 8, m_Height / 8, 7 * m_Width / 8, 7 * m_Height / 8);

			dc.MoveTo(0, 0, NULL);
			dc.LineTo(m_Width, m_Height);

			dc.MoveTo(0, m_Height, NULL);
			dc.LineTo(m_Width, 0);

			dc.Ellipse(m_Width / 8, m_Height / 8, 7 * m_Width / 8, 7 * m_Height / 8);
			dc.RoundRect(m_Width / 4, m_Height / 4, 3 * m_Width / 4, 3 * m_Height / 4, m_Width / 4, m_Height / 4);
		}
		else if (m_drawType == DRAW_TYPE_BEZIER)
		{
			//With PolyBezier, the first four points indicate (in this order) the begin point, first control point, second control point, and end point of the first Bezier curve. 
			//Each subsequent Bezier requires only three more points because the begin point of the second Bezier curve is the same as the end point of the first Bezier curve, and so on
			//3n+1 in all
			//dc.PolyBezier(bezier_apt, 4);

			//The first and each subsequent Bezier spline requires only three points.
			//dc.PolyBezierTo(bezier_apt+1, 3);

			DrawBezier(dc.m_hDC, bezier_apt);
		}
		else if (m_drawType == DRAW_TYPE_PEN)
		{
			dc.SelectPen(CreatePen(PS_SOLID, 0, RGB(255, 0, 0)));
			dc.MoveTo(100, 10, NULL);
			dc.LineTo(300, 10);

			DeleteObject(dc.SelectPen(CreatePen(PS_DASH, 0, RGB(255, 0, 0))));
			dc.MoveTo(100, 20, NULL);
			dc.LineTo(300, 20);

			DeleteObject(dc.SelectPen(CreatePen(PS_DOT, 0, RGB(255, 0, 0))));
			dc.MoveTo(100, 30, NULL);
			dc.LineTo(300, 30);

			DeleteObject(dc.SelectPen(CreatePen(PS_DASHDOT, 0, RGB(255, 0, 0))));
			dc.MoveTo(100, 40, NULL);
			dc.LineTo(300, 40);

			DeleteObject(dc.SelectPen(CreatePen(PS_DASHDOTDOT, 0, RGB(255, 0, 0))));
			dc.MoveTo(100, 50, NULL);
			dc.LineTo(300, 50);

			DeleteObject(dc.SelectPen(CreatePen(PS_NULL, 0, RGB(255, 0, 0))));
			dc.MoveTo(100, 60, NULL);
			dc.LineTo(300, 60);

			DeleteObject(dc.SelectPen(CreatePen(PS_INSIDEFRAME, 0, RGB(255, 0, 0))));
			dc.MoveTo(100, 70, NULL);
			dc.LineTo(300, 70);

			DeleteObject(dc.SelectPen(CreatePen(PS_DOT, 0, RGB(255, 0, 0))));
			dc.SetBkColor(RGB(0, 255, 0));
			dc.MoveTo(100, 80, NULL);
			dc.LineTo(300, 80);

			DeleteObject(dc.SelectPen(CreatePen(PS_DOT, 0, RGB(255, 0, 0))));
			dc.SetBkMode(TRANSPARENT);
			dc.MoveTo(100, 90, NULL);
			dc.LineTo(300, 90);
			DeleteObject(dc.SelectPen(D_BLACK_PEN));
		}
		else if (m_drawType == DRAW_TYPE_BRUSH)
		{
			dc.SelectBrush(CreateHatchBrush(HS_HORIZONTAL, RGB(255, 0, 0)));
			dc.Rectangle(100, 100, 130, 130);

			DeleteObject(dc.SelectBrush(CreateHatchBrush(HS_VERTICAL, RGB(255, 0, 0))));
			dc.Rectangle(140, 100, 170, 130);

			DeleteObject(dc.SelectBrush(CreateHatchBrush(HS_CROSS, RGB(255, 0, 0))));
			dc.Rectangle(180, 100, 210, 130);

			DeleteObject(dc.SelectBrush(CreateHatchBrush(HS_BDIAGONAL, RGB(255, 0, 0))));
			dc.Rectangle(100, 140, 130, 170);

			DeleteObject(dc.SelectBrush(CreateHatchBrush(HS_FDIAGONAL, RGB(255, 0, 0))));
			dc.Rectangle(140, 140, 170, 170);

			DeleteObject(dc.SelectBrush(CreateHatchBrush(HS_DIAGCROSS, RGB(255, 0, 0))));
			dc.Rectangle(180, 140, 210, 170);

			//TODO: 
			//BS_PATTERN and BS_DIBPATTERNPT

			DeleteObject(dc.SelectBrush(D_WHITE_BRUSH));
		}
		else if (m_drawType == DRAW_TYPE_WINDING)
		{
			static POINT aptFigure[10] = { 10,70, 50,70, 50,10, 90,10, 90,50,
				30,50, 30,90, 70,90, 70,30, 10,30 };

			dc.SelectBrush(D_GRAY_BRUSH);

			for (int i = 0; i < 10; i++)
			{
				wind_apt[i].x = m_Width * aptFigure[i].x / 200;
				wind_apt[i].y = m_Height * aptFigure[i].y / 100;
			}

			dc.SetPolyFillMode(ALTERNATE);
			dc.Polygon(wind_apt, 10);

			for (int i = 0; i < 10; i++)
			{
				wind_apt[i].x += m_Width / 2;
			}

			dc.SetPolyFillMode(WINDING);
			dc.Polygon(wind_apt, 10);
		}
		else if (m_drawType == DRAW_TYPE_PENTAGRAM)
		{
			/*DPoint p[5], p2[5];
			DRender::GenPentagramData(DPoint(200,300),100, p);
			DRender::GenPentagramData(DPoint(450,300),100, p2);
			dc.SelectBrush(D_GRAY_BRUSH);
			dc.SetPolyFillMode(ALTERNATE);
			dc.Polygon((POINT*)p,5);
			dc.SetPolyFillMode(WINDING);
			dc.Polygon((POINT*)p2,5);*/
		}
		else if (m_drawType == DRAW_TYPE_MAPMODE)
		{
			static TCHAR szHeading[] = TEXT("Mapping Mode            Left   Right     Top  Bottom");
			static TCHAR szUndLine[] = TEXT("------------            ----   -----     ---  ------");

			dc.SetMapMode(MM_LOENGLISH);
			dc.TextOut(50, -100, TEXT("Hello"), 5);

			dc.SelectFont(D_SYSTEM_FIXED_FONT);

			dc.SetMapMode(MM_ANISOTROPIC);
			dc.SetWindowExt(1, 1, NULL);
			dc.SetViewportExt(m_cxChar, m_cyChar, NULL);

			dc.TextOut(1, 1, szHeading, lstrlen(szHeading));
			dc.TextOut(1, 2, szUndLine, lstrlen(szUndLine));

			Show(m_hWnd, dc, 1, 3, MM_TEXT, TEXT("TEXT (pixels)"));
			Show(m_hWnd, dc, 1, 4, MM_LOMETRIC, TEXT("LOMETRIC (.1 mm)"));
			Show(m_hWnd, dc, 1, 5, MM_HIMETRIC, TEXT("HIMETRIC (.01 mm)"));
			Show(m_hWnd, dc, 1, 6, MM_LOENGLISH, TEXT("LOENGLISH (.01 in)"));
			Show(m_hWnd, dc, 1, 7, MM_HIENGLISH, TEXT("HIENGLISH (.001 in)"));
			Show(m_hWnd, dc, 1, 8, MM_TWIPS, TEXT("TWIPS (1/1440 in)"));
		}
		else if (m_drawType == DRAW_TYPE_CLOVER)
		{
			double fAngle, fRadius;
			dc.SetViewportOrg(m_Width / 2, m_Height / 2, NULL);
			dc.SelectClipRgn(hRgnClip);

			fRadius = _hypot(m_Width / 2.0, m_Height / 2.0);

			for (fAngle = 0.0; fAngle < TWOPI; fAngle += TWOPI / 360)
			{
				dc.MoveTo(0, 0, NULL);
				dc.LineTo((int)(fRadius * cos(fAngle) + 0.5), (int)(-fRadius * sin(fAngle) + 0.5));
			}
		}
		return 0;
	}

	//Focus
	void OnSetFocus(HWND wndOld)
	{
		CreateCaret(NULL, 0, m_cyChar);
		::SetCaretPos(5, 105);
		ShowCaret();
	}

	void OnKillFocus(HWND wndFocus)
	{
		HideCaret();
		DestroyCaret();
	}

	//Mouse
	void OnMouseProcess(UINT nFlags, DPoint point)
	{
		if (m_drawType != DRAW_TYPE_BEZIER) return;
		if ((nFlags & MK_LBUTTON || nFlags & MK_RBUTTON))
		{
			DClientDC dc(m_hWnd);
			dc.SelectPen(D_WHITE_PEN);
			DrawBezier(dc.m_hDC, bezier_apt);

			if (nFlags & MK_LBUTTON)
			{
				bezier_apt[1].x = point.x;
				bezier_apt[1].y = point.y;
			}

			if (nFlags & MK_RBUTTON)
			{
				bezier_apt[2].x = point.x;
				bezier_apt[2].y = point.y;
			}

			dc.SelectPen(D_BLACK_PEN);
			DrawBezier(dc.m_hDC, bezier_apt);
		}
	}

	//Keyboard
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		m_Input += (wchar_t)nChar;
	}

	void OnInputLangChange(DWORD dwCharSet, HKL hKbdLayout)
	{
		m_dwCharSet = dwCharSet;

		DClientDC dc(m_hWnd);

		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		m_cxChar = tm.tmAveCharWidth;
		m_cyChar = tm.tmHeight;
	}

	//Menus
	//System
	LRESULT OnReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_NONE;
		m_drawText = 0;
		DClientDC dc(m_hWnd);
		dc.SetViewportOrg(0, 0, NULL);
		dc.SelectClipRgn(NULL);
		dc.SetMapMode(MM_TEXT);
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnInFalse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_erased = 0;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnInTrue(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_erased = 1;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT DumpSysMetric(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DString str = DWinDbg::GetSysMetricStr();
		MessageBox(str, L"System Metric", MB_OK);
		return 0;
	}

	LRESULT DumpDCCaps(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DClientDC dc(m_hWnd);
		DString str = DWinDbg::GetDCCapsStr(dc);
		MessageBox(str, L"Device Caps", MB_OK);
		return 0;
	}

	LRESULT DumpDCAttr(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DClientDC dc(m_hWnd);
		DString str = DWinDbg::GetDCAttrStr(dc);
		MessageBox(str, L"Device Attribute", MB_OK);
		return 0;
	}


	LRESULT OnDumpStyle(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DString strMsg;

		DWORD dwStyle = GetStyle();
		DString str = DWinDbg::WindowStyleToStr((DUInt32)dwStyle);
		strMsg += str;
		strMsg += L"\r\n";

		DWORD dwExStyle = GetExStyle();
		str = DWinDbg::WindowExStyleToStr((DUInt32)dwExStyle);
		strMsg += str;
		strMsg += L"\r\n";

		MessageBox(strMsg, L"Window Style", MB_OK);
		return 0;
	}

	LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DestroyWindow();
		return 0;
	}

	//Lines
	LRESULT OnDrawGrid(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_GRID;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawPolygon(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_POLYGON;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawSine(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_SINE;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawRect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_RECT;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawEllipse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_ELLIPSE;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawRoundRect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_ROUNDRECT;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawArc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_ARC;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawChord(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_CHORD;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawPie(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_PIE;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawDemo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_DEMO;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawBezier(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_BEZIER;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawPen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_PEN;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	void DrawBezier(DDCHandle dc, POINT apt[])
	{
		dc.PolyBezier(apt, 4);
		dc.MoveTo(apt[0].x, apt[0].y, NULL);
		dc.LineTo(apt[1].x, apt[1].y);
		dc.MoveTo(apt[2].x, apt[2].y, NULL);
		dc.LineTo(apt[3].x, apt[3].y);
	}

	//Rect
	LRESULT OnDrawBrush(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_BRUSH;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}
	
	LRESULT OnDrawPentaGram(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_PENTAGRAM;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawWinding(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_WINDING;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawMapMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_MAPMODE;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnDrawClover(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawType = DRAW_TYPE_CLOVER;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnSetMapMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DClientDC dc(m_hWnd);
		if (wID == ID_MMTEXT) dc.SetMapMode(MM_TEXT);
		else if (wID == ID_MMLOMETRIC) dc.SetMapMode(MM_LOMETRIC);
		else if (wID == ID_MMLOENGLISH) dc.SetMapMode(MM_LOENGLISH);
		else if (wID == ID_MMHIMETRIC) dc.SetMapMode(MM_HIMETRIC);
		else if (wID == ID_MMHIENGLISH) dc.SetMapMode(MM_HIENGLISH);
		else if (wID == ID_MMTWIPS) dc.SetMapMode(MM_TWIPS);
		else if (wID == ID_MMISOTROPIC) dc.SetMapMode(MM_ISOTROPIC);
		else if (wID == ID_ANISOTROPIC) dc.SetMapMode(MM_ANISOTROPIC);
		return 0;
	}

	void Show(HWND hwnd, HDC hdc, int xText, int yText, int iMapMode, TCHAR* szMapMode)
	{
		TCHAR szBuffer[60];
		RECT  rect;

		::SaveDC(hdc);

		::SetMapMode(hdc, iMapMode);
		::GetClientRect(hwnd, &rect);
		::DPtoLP(hdc, (PPOINT)& rect, 2);

		::RestoreDC(hdc, -1);

		::TextOut(hdc, xText, yText, szBuffer,
			DStdLib::swnprintf(szBuffer, 60, L"%-20s %7d %7d %7d %7d", szMapMode, rect.left, rect.right, rect.top, rect.bottom));
	}
	
	//Text
	LRESULT DumpTextMetrics(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DClientDC dc(m_hWnd);
		DString str = DWinDbg::GetTextMetricsStr(dc);
		MessageBox(str, L"Text Metrics", MB_OK);
		return 0;
	}

	LRESULT OnTextOut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawText = 1;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	LRESULT OnPrintInput(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_drawText = 2;
		::InvalidateRect(m_hWnd, NULL, TRUE);
		return 0;
	}

	//Window
	LRESULT OnOpenKeyboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		RECT r = {0,0,600,600};
		DPRect dr(&r);
		DHMenu dm;
		kwin.Create(NULL, dr, dm, L"DKeyboard Window", WS_OVERLAPPEDWINDOW|WS_VISIBLE, WS_EX_STATICEDGE);
		kwin.CenterWindow();
		kwin.ShowWindow(SW_SHOW);
		kwin.UpdateWindow();
		return 0;
	}

	LRESULT OnOpenMouse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		RECT r = { 0,0,600,600 };
		DPRect dr(&r);
		DHMenu dm;
		mwin.Create(NULL, dr, dm, L"DMouse Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, WS_EX_STATICEDGE);
		mwin.CenterWindow();
		mwin.ShowWindow(SW_SHOW);
		mwin.UpdateWindow();
		return 0;
	}

	LRESULT OnOutputConsole(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		console.Output(L"Hello World,(%d * %d)\n", m_Width, m_Height);

		console.Output(L"sizeof(DWnd)=%u\n", DSizeOf(DWnd));
		console.Output(L"sizeof(DMsgMap)=%u\n", DSizeOf(DMsgHandler));
		console.Output(L"sizeof(DWndProcThunk)=%u\n", DSizeOf(DWndProcThunk));
		console.Output(L"sizeof(DControlWinTraits)=%u\n", DSizeOf(DControlWinTraits));
		console.Output(L"sizeof(DVScrollFrameWinTraits)=%u\n", DSizeOf(DVScrollFrameWinTraits));

		console.Output(L"sizeof(DMSG)=%u\n", DSizeOf(DMSG));
		console.Output(L"sizeof(DWndImplRoot<DWnd>)=%u\n", DSizeOf(DWndImplRoot<DWnd>));
		console.Output(L"sizeof(DWndImplBaseT<DWnd>)=%u\n", DSizeOf(DWndImplBaseT<DWnd>));

		console.Output(L"sizeof(DWinApp)=%u\n", DSizeOf(DWinApp));
		console.Output(L"sizeof(DCriticalSection)=%u\n", DSizeOf(DCriticalSection));
		console.Output(L"sizeof(DPRect)=%u\n", DSizeOf(DPRect));
		console.Output(L"sizeof(DHMenu)=%u\n", DSizeOf(DHMenu));
		console.Output(L"sizeof(DResStr)=%u\n", DSizeOf(DResStr));

		console.Output(L"sizeof(DCreateWndData)=%u\n", DSizeOf(DCreateWndData));
		console.Output(L"sizeof(DWndClassInfo)=%u\n", DSizeOf(DWndClassInfo));

		console.Output(L"sizeof(_dthunk)=%u\n", DSizeOf(_dthunk));
		console.Output(L"sizeof(DThunk)=%u\n", DSizeOf(DThunk));

		return 0;
	}

	//DWinUI DWinApp
	LRESULT OnWndClassTest(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		int nCount = 0;
		ATOM* pAtom = DWinApp::GetRegWindowAtoms(&nCount);
		for (int i = 0; i < nCount; i++)
		{
			DString str = DWinDbg::ATOMToStr(pAtom[i]);
			DString strTitle;
			strTitle.Format(L"ATOM:%d (%d/%d)", pAtom[i], i+1, nCount);
			DMessageBox(m_hWnd, (LPCWSTR)str.GetStr(), (LPCWSTR)strTitle.GetStr());
		}
		return 0;
	}
	
	//DWnd
	LRESULT OnDWndTest(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DWnd::Test();
		return 0;
	}

	LRESULT OnDContainedWndTest(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		RECT r = { 300,300,940,780 };
		DHMenu dm;
		cwin.Create(NULL, &r, dm, L"DContainWindow Test Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, WS_EX_STATICEDGE);
		cwin.ShowWindow(SW_SHOW);
		cwin.UpdateWindow();
		return 0;
	}

	LRESULT OnLoadAndDraw(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DClientDC dc(m_hWnd);
		DData* pData = DBmpFile::Load("..\\..\\Data\\Image\\24bit.bmp", nullptr);
		DUInt32 width = pData->GetUInt32(DBMP_UINT32_WIDTH);
		DUInt32 height = pData->GetUInt32(DBMP_UINT32_HEIGHT);
		/*for (int x = 0; x < (int)width; x++) {
			for (int y = 0; y < (int)height; y++) {
				COLORREF cr = DBmpFile::GetRGB(pData, x, y);
				dc.SetPixel(x, height - y, cr);
			}
		}*/
		DBuffer bufInfo = pData->GetBuffer(DBMP_BUFFER_INFO_BUFFER);
		DBuffer bufData = pData->GetBuffer(DBMP_BUFFER_DATA_BUFFER);
		dc.StretchDIBits(0, 0, width, height, 0, 0, width, height, 
			bufData.GetBuf(), (const BITMAPINFO *) bufInfo.GetBuf(), DIB_RGB_COLORS, SRCCOPY);

		return 0;
	}

	LRESULT OnLoadJpeg(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DClientDC dc(m_hWnd);
		DQuickTable::InitLogFiles();
		DQuickTable::BuildYuvRGBTable();
		//DData* pData = DJpegFile::Decode("..\\..\\Data\\Image\\bg.jpg");
		DData* pData = DJpegFile::Decode("..\\..\\Data\\Image\\jpeg.jpg");
		DUInt32 width = pData->GetUInt32(0);
		DUInt32 height = pData->GetUInt32(1);
		for (DUInt32 y = 0; y < height; y++) {
			for (DUInt32 x = 0; x < width; x++) {
				COLORREF cr = DJpegFile::GetRGB(pData, x, y);
				dc.SetPixel(x, y, cr);
			}
		}
		/*
		for (DUInt32 ii = 0; ii < width; ii++) {
			for (DUInt32 jj = 0; jj < height; jj++) {
				DUInt32 tt = jj * width + ii;
				DBuffer buf = pData->GetBuffer(tt+2);
				for (DUInt32 i = 0; i < 16; i++) {
					for (DUInt32 j = 0; j < 16; j++) {
						DUInt32 pos = (i * 16 + j) * 3;
						COLORREF cr = RGB(buf.GetAt(pos + 2), buf.GetAt(pos + 1), buf.GetAt(pos));
						dc.SetPixel(ii * 16 + j, jj * 16 + i, cr);
					}
				}
			}
		}*/
		return 0;
	}

	//CommDlg
	LRESULT OnOpenFileDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DFileDialog::TestOpen(this);
		return 0;
	}

	LRESULT OnSaveFileDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DFileDialog::TestSave(this);
		return 0;
	}

	LRESULT OnSimpleDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DSimpleFileDialog::Test(this);
		return 0;
	}
	
	LRESULT OnMultiFileDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DMultiFileDialog::Test(this);
		return 0;
	}

	LRESULT OnShellOpenDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DShellFileOpenDialog::Test(this);
		return 0;
	}

	LRESULT OnShellSaveDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DShellFileSaveDialog::Test(this);
		return 0;
	}

	LRESULT OnFolderDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DFolderDialog::Test(this);
		return 0;
	}

	LRESULT OnFontDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DFontDialog::Test(this);
		return 0;
	}

	LRESULT OnRichEditFontDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DRichEditFontDialog::Test(this);
		return 0;
	}

	LRESULT OnColorDialog(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		DColorDialog::Test(this);
		return 0;
	}

	LRESULT OnOpenSplitterWnd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		RECT r = { 300,300,940,780 };
		DHMenu dm;
		swin.Create(NULL, &r, dm, L"DSplitWindow Test Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, WS_EX_STATICEDGE);
		swin.ShowWindow(SW_SHOW);
		swin.UpdateWindow();
		return 0;
	}

	LRESULT OnOpenScrollWnd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		RECT r = { 300,300,940,780 };
		DHMenu dm;
		scwin.Create(NULL, &r, dm, L"DScrollWindow Test Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, WS_EX_STATICEDGE);
		scwin.ShowWindow(SW_SHOW);
		scwin.UpdateWindow();
		return 0;
	}
	
	//Final
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		PostQuitMessage(0);
	}

private:
	DInt32 m_erased;
	DInt32 m_Height;
	DInt32 m_Width;
	DInt32 m_drawType;
	DInt32 m_drawText;
	DString strPaintInfo;

	POINT bezier_apt[4];
	POINT wind_apt[10];

	HRGN hRgnClip;
	HRGN hRgnTemp[6];

	//Input
	DString m_Input;
	DUInt32 m_dwCharSet;
	DUInt32 m_cxChar;
	DUInt32 m_cyChar;
	DFont	m_font;

	DKeybdWindow kwin;
	DMouseWindow mwin;
	DContainWindow cwin;
	DSplitWindow swin;
	DScrollWindow scwin;
	DFrameWindow fwin;
	DConsole console;
};

