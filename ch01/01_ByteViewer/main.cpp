// Main.cpp : Defines the entry point for the application.
//

#include "Base/DMemAlloc.h"
#include "WinExtra/DWinApp.h"
#include "resource.h"
#include "MainDlg.h"


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	D_UNUSED(hPrevInstance);
	D_UNUSED(lpCmdLine);
	DMemAlloc::Init();
	DWinApp::SetAppInstance(hInstance);

	DMainDialog<IDD_MAINDIALOG, TRUE> dlg;
	dlg.DoModal();

	DMessageLoop* loop = DMessageLoop::GetCurrentLoop();
	loop->Run();
	return 0;
}
