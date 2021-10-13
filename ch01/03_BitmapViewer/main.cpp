// Main.cpp : Defines the entry point for the application.
//

#include "DConfig.h"
#include "DTypes.h"
#include "WinExtra/DWinApp.h"
#include "DMainWindow.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	D_UNUSED(hPrevInstance);
	D_UNUSED(lpCmdLine);
	
	DWinApp::SetAppInstance(hInstance);
	HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));

	DMainWindow wnd;
	RECT r = {300,300,940,780};
	DPRect dr(&r);
	DHMenu dm(hMenu);
	wnd.Create(NULL, dr, dm, L"DWinUI Test Window", WS_OVERLAPPEDWINDOW|WS_VISIBLE, WS_EX_STATICEDGE);
	wnd.ShowWindow(nCmdShow);
	wnd.UpdateWindow();

	DMessageLoop *loop = DMessageLoop::GetCurrentLoop();
	loop->Run();

	return 0;
}

