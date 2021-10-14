#pragma once

#include "DTypes.h"
#include "resource.h"
#include "atlwinx.h"
#include "atlcrack.h"
#include "atlmisc.h"
#include "atlctrls.h"
#include "atlgdi.h"

class CMainWindow : public CWindowImpl<CMainWindow>
{
public:
    DECLARE_WND_CLASS(L"RTCMainWin")

public:
    CMainWindow()
    {

    }

    BEGIN_MSG_MAP(CMainWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        MSG_WM_VSCROLL(OnVScroll)
        MSG_WM_HSCROLL(OnHScroll) 
        COMMAND_ID_HANDLER(ID_BITMAP_LOADANDDRAW, OnLoadAndDraw)
    END_MSG_MAP()

    //Create
    int OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
        return 0;
    }

    //Size
    void OnSize(UINT nType, CSize size)
    {
        m_Width = size.cx;
        m_Height = size.cy;

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

        HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        ShowCursor(TRUE);
    }

    DVoid OnVScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar)
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

    DVoid OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar)
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

    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        CPaintDC dc(m_hWnd);
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
        return 0;
    }


    LRESULT OnLoadAndDraw(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {/*
        DClientDC dc(m_hWnd);
        DData* pData = DBmpFile::Load("..\\..\\Data\\Image\\24bit.bmp", nullptr);
        DUInt32 width = pData->GetUInt32(DBMP_UINT32_WIDTH);
        DUInt32 height = pData->GetUInt32(DBMP_UINT32_HEIGHT);
        for (int x = 0; x < (int)width; x++) {
            for (int y = 0; y < (int)height; y++) {
                COLORREF cr = DBmpFile::GetRGB(pData, x, y);
                dc.SetPixel(x, height - y, cr);
            }
        }
        DBuffer bufInfo = pData->GetBuffer(DBMP_BUFFER_INFO_BUFFER);
        DBuffer bufData = pData->GetBuffer(DBMP_BUFFER_DATA_BUFFER);
        dc.StretchDIBits(0, 0, width, height, 0, 0, width, height,
            bufData.GetBuf(), (const BITMAPINFO*)bufInfo.GetBuf(), DIB_RGB_COLORS, SRCCOPY);
            */
        return 0;
    }

    virtual void OnFinalMessage(HWND /*hWnd*/)
    {
        PostQuitMessage(0);
    }

private:
    DInt32 m_erased;
    DInt32 m_Height;
    DInt32 m_Width;
};

