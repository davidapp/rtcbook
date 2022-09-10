#pragma once

#include "DTypes.h"
#include "resource.h"
#include "atlwinx.h"
#include "atlcrack.h"
#include "atlmisc.h"
#include "atlctrls.h"
#include "atlgdi.h"
#include "Video/WinDSVideoCapture.h"


class CMainWindow : public CWindowImpl<CMainWindow>, public WinDSVideoCaptureSink
{
public:
    DECLARE_WND_CLASS(L"RTCMainWin")

public:
    CMainWindow()
    {
        m_Height = 0;
        m_Width = 0;
    }

    BEGIN_MSG_MAP(CMainWindow)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_SIZE(OnSize)
        COMMAND_ID_HANDLER(ID_CAMERA_START, OnCameraStart)
        COMMAND_ID_HANDLER(ID_CAMERA_STOP, OnCameraStop)
    END_MSG_MAP()

    virtual DVoid OnFrame(const DVideoFormat& frame) {
        
    }

    virtual DVoid OnError(DUInt32 errorCode) {
        
    }

    int OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
        CenterWindow();
        m_vcap.SetSink(this);
        return 0;
    }

    void OnSize(UINT nType, CSize size)
    {
        m_Width = size.cx;
        m_Height = size.cy;
    }

    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        CPaintDC dc(m_hWnd);
        RECT r;
        GetClientRect(&r);

        return 0;
    }

    LRESULT OnCameraStart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        m_vcap.Start();
        return 0;
    }

    LRESULT OnCameraStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        m_vcap.Stop();
        return 0;
    }

    virtual void OnFinalMessage(HWND /*hWnd*/)
    {
        PostQuitMessage(0);
    }

private:
    DInt32 m_Height;
    DInt32 m_Width;
    WinDSVideoCapture m_vcap;
};

