#pragma once

#include "atl.h"
#include "atldlgs.h"
#include "atlctrls.h"
#include "atlmisc.h"
#include "atlcrack.h"
#include "resource.h"
#include "atlgdi.h"
#include "Base/DFile.h"
#include "Base/DPath.h"
#include "Video/DVideoFrame.h"
#include "Video/DVideoI420.h"
#include "Video/DVideoFormat.h"

// For OpenGL
#include <GL/GL.h>

// For DirectDraw
#include <ddraw.h>
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")


LPDIRECTDRAW7        g_pDD = NULL;        // DirectDraw object
LPDIRECTDRAWSURFACE7 g_pDDSFront = NULL;  // DirectDraw fronbuffer surface
LPDIRECTDRAWSURFACE7 g_pDDSBack = NULL;   // DirectDraw backbuffer surface

int InitDirectDraw(HWND hwnd)
{
    DirectDrawCreateEx(NULL, (VOID**)&g_pDD, IID_IDirectDraw7, NULL);
    g_pDD->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
    g_pDD->SetDisplayMode(300, 300, 32, 0, 0);

    DDSURFACEDESC2 ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    ddsd.dwBackBufferCount = 1;
    g_pDD->CreateSurface(&ddsd, &g_pDDSFront, NULL);

    DDSCAPS2 ddscaps;
    ZeroMemory(&ddscaps, sizeof(ddscaps));
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    g_pDDSFront->GetAttachedSurface(&ddscaps, &g_pDDSBack);

    return 0;
}

void CleanUp()
{
    if (g_pDDSBack)
        g_pDDSBack->Release();

    if (g_pDDSFront)
        g_pDDSFront->Release();

    if (g_pDD)
        g_pDD->Release();
}

class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter
{
public:
    enum { IDD = IDD_DIALOG1 };

    CMainDlg()
    {
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        return ::IsDialogMessage(m_hWnd, pMsg);
    }

    BEGIN_MSG_MAP(CSettingDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_RANGE_HANDLER(IDOK, IDNO, OnCloseCmd)
        COMMAND_ID_HANDLER(IDC_GDI, OnGDI)
        COMMAND_ID_HANDLER(IDC_DBUFFER, OnDoubleBuffer)
        COMMAND_ID_HANDLER(IDC_OPENGL, OnOpenGL)
        COMMAND_ID_HANDLER(IDC_OPENGL2, OnOpenGL2)
        COMMAND_ID_HANDLER(IDC_DIRECTDRAW, OnDirectDraw)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());
        std::string appdir = DPath::GetExeDir();
        appdir = appdir.substr(0, appdir.size() - 2);
        std::string datafile = DPath::GetParent(appdir) + "Data\\I420Frame\\I420Frame.vf";
        DBuffer bufFrame = DFile::FilePath2Buffer(datafile.c_str());
        m_frame.LoadFromBuffer(bufFrame);

        m_destRect.top = 0;
        m_destRect.left = 0;
        m_destRect.bottom = 300;
        m_destRect.right = 300;

        m_showwindow = GetDlgItem(IDC_SHOW);
        m_showwindow.MoveWindow((LPCRECT) & m_destRect);

        InitDirectDraw(m_hWnd);
        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        ::EndDialog(m_hWnd, wID);
        m_hWnd = NULL;
        PostQuitMessage(0);
        return 0;
    }

    LRESULT OnGDI(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        CDC dc = ::GetDC(m_hWnd);
        DVideoFrame i420frame_e = DVideoI420::Scale(m_frame, m_destRect.Width(), m_destRect.Height(), kFilterBox);
        DVideoFrame frameRaw = DVideoFormat::I420ToRAW(i420frame_e);
        DRect src = DRect(0, 0, i420frame_e.GetWidth(), i420frame_e.GetHeight());
        DBITMAPINFOHEADER* pHrd = frameRaw.NewBMPInfoHeader();

        HBRUSH brush1 = ::CreateSolidBrush(RGB(0, 0, 255));
        RECT logical_rect1 = { 0, 0, m_destRect.Width(), m_destRect.Height() - 1};
        dc.FillRect(&logical_rect1, brush1);
        ::DeleteObject(brush1);
        
        dc.StretchDIBits(0, 0, m_destRect.Width(), m_destRect.Height(),
            0, src.Height(), src.Width(), -src.Height(), frameRaw.GetBuf(),
            (const BITMAPINFO*)pHrd, DIB_RGB_COLORS, SRCCOPY);
        
        delete pHrd;
        return 0;
    }

    LRESULT OnDoubleBuffer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        CDC dc = ::GetDC(m_hWnd);
        DVideoFrame i420frame_e = DVideoI420::Scale(m_frame, m_destRect.Width(), m_destRect.Height(), kFilterBox);
        DVideoFrame frameRaw = DVideoFormat::I420ToRAW(i420frame_e);
        DRect src = DRect(0, 0, i420frame_e.GetWidth(), i420frame_e.GetHeight());
        DBITMAPINFOHEADER* pHrd = frameRaw.NewBMPInfoHeader();

        CDC dc_mem;
        dc_mem.CreateCompatibleDC(dc.m_hDC);

        HBITMAP bmp_mem = ::CreateCompatibleBitmap(dc.m_hDC, m_destRect.Width(), m_destRect.Height());
        HGDIOBJ bmp_old = ::SelectObject(dc_mem.m_hDC, bmp_mem);
        HBRUSH brush = ::CreateSolidBrush(RGB(0, 0, 255));
        RECT logical_rect = { 0, 0, m_destRect.Width(), m_destRect.Height() - 1 };
        ::FillRect(dc_mem, &logical_rect, brush);
        ::DeleteObject(brush);

        dc_mem.StretchDIBits(0, 0, m_destRect.Width(), m_destRect.Height(),
            0, src.Height(), src.Width(), -src.Height(), frameRaw.GetBuf(),
            (const BITMAPINFO*)pHrd, DIB_RGB_COLORS, SRCCOPY);

        dc.BitBlt(m_destRect.left, m_destRect.top, m_destRect.Width(), m_destRect.Height() - 1, dc_mem, 0, 0, SRCCOPY);

        ::SelectObject(dc_mem, bmp_old);
        ::DeleteObject(bmp_mem);
        ::DeleteDC(dc_mem);

        delete pHrd;
        return 0;
    }

    LRESULT OnOpenGL(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        HDC hDC;
        HGLRC hRC;
        EnableOpenGL(m_showwindow.m_hWnd, &hDC, &hRC);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glPushMatrix();
        glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f); glVertex2f(0.0f, 1.0f);
        glColor3f(0.0f, 1.0f, 0.0f); glVertex2f(0.87f, -0.5f);
        glColor3f(0.0f, 0.0f, 1.0f); glVertex2f(-0.87f, -0.5f);
        glEnd();
        glPopMatrix();

        SwapBuffers(hDC);
        DisableOpenGL(m_showwindow.m_hWnd, hDC, hRC);
        return 0;
    }

    LRESULT OnOpenGL2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        DVideoFrame i420frame_e = DVideoI420::Scale(m_frame, m_destRect.Width(), m_destRect.Height(), kFilterBox);

        HDC hDC;
        HGLRC hRC;
        EnableOpenGL(m_showwindow.m_hWnd, &hDC, &hRC);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        GLuint texture1_ = 0;
        GLuint texture2_ = 0;
        GLuint texture3_ = 0;
        glGenTextures(1, &texture1_);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, i420frame_e.GetWidth(),
            i420frame_e.GetHeight(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, i420frame_e.GetBuf());
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1_);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glViewport(0, 0, 200, 200);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SwapBuffers(hDC);
        DisableOpenGL(m_showwindow.m_hWnd, hDC, hRC);
        return 0;
    }

    LRESULT OnDirectDraw(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        m_pSurface->Restore();

        DDSURFACEDESC2 ddsd;
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
        m_pSurface->GetSurfaceDesc(&ddsd);


        return 0;
    }

    DVoid EnableOpenGL(HWND hWnd, HDC* hDC, HGLRC* hRC)
    {
        *hDC = ::GetDC(hWnd);

        PIXELFORMATDESCRIPTOR pfd;
        ZeroMemory(&pfd, sizeof(pfd));
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW |
            PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 16;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int iFormat = ChoosePixelFormat(*hDC, &pfd);
        SetPixelFormat(*hDC, iFormat, &pfd);

        *hRC = wglCreateContext(*hDC);
        wglMakeCurrent(*hDC, *hRC);
    }

    DVoid DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hRC);
        ::ReleaseDC(hWnd, hDC);
    }

    DBool CreateSurface(LPDIRECTDRAW7 hDD, int nWidth, int nHeight, COLORREF dwColorKey)
    {
        DDSURFACEDESC2 ddsd;
        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
        ddsd.dwWidth = nWidth;
        ddsd.dwHeight = nHeight;

        HRESULT hRet;
        hDD->CreateSurface(&ddsd, &m_pSurface, NULL);
        if (hRet != DD_OK)
        {
            if (hRet == DDERR_OUTOFVIDEOMEMORY)
            {
                ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |
                    DDSCAPS_SYSTEMMEMORY;

                hRet = hDD->CreateSurface(&ddsd, &m_pSurface, NULL);
            }

            if (hRet != DD_OK)
            {
                return FALSE;
            }
        }

        DDCOLORKEY ddck;
        if ((int)dwColorKey != -1)
        {
            ddck.dwColorSpaceLowValue = dwColorKey;
            ddck.dwColorSpaceHighValue = 0;
            m_pSurface->SetColorKey(DDCKEY_SRCBLT, &ddck);
        }

        m_ColorKey = dwColorKey;
        m_Width = nWidth;
        m_Height = nHeight;
    }

    DVideoFrame m_frame;
    CStatic m_showwindow;
    DRect m_destRect;

    // DDraw
    COLORREF m_ColorKey;
    UINT m_Height;
    UINT m_Width;
    LPDIRECTDRAWSURFACE7 m_pSurface;
};
