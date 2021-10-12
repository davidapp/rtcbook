// Main.cpp : Defines the entry point for the application.
//
#include "WinExtra/DWinApp.h"
#include "DMainDlg.h"
#include "Base/DMemAlloc.h"
#include "resource.h"

#define D_USE_DLG 1

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	D_UNUSED(hPrevInstance);
	D_UNUSED(lpCmdLine);
	DMemAlloc::Init();
	DWinApp::SetAppInstance(hInstance);

#if defined(D_USE_DLG) && (D_USE_DLG==1)
	DMainDialog<IDD_MAINDIALOG, TRUE> dlg;
	dlg.DoModal();
#else
	HMENU hMenu = NULL;
	DMainWin wnd;
	RECT r = { 300,300,540,540 };
	wnd.Create(NULL, DPRect(&r), DHMenu(hMenu), L"DTextViewer", WS_OVERLAPPEDWINDOW | WS_VISIBLE, WS_EX_STATICEDGE);
	wnd.CenterWindow();
	wnd.ShowWindow(SW_SHOW);
	wnd.UpdateWindow();
#endif

	DMessageLoop* loop = DMessageLoop::GetCurrentLoop();
	loop->Run();
	return 0;
}

