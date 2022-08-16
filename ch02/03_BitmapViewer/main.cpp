#include "atl.h"
#include "atldlgs.h"
#include "resource.h"
#include "MainWindow.h"

CAppModule _Module;

int Run(LPTSTR /*lpCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CMainWindow winMain;

    if (winMain.Create(NULL, (LPRECT)CRect(0,0,800,600), L"Bitmap Viewer",
        WS_VISIBLE | WS_OVERLAPPEDWINDOW, 0, ::LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU1))) == NULL)
    {
        ATLTRACE(_T("Main window creation failed!\n"));
        return 0;
    }

    winMain.ShowWindow(nCmdShow);
    winMain.UpdateWindow();

    int nRet = theLoop.Run();

    _Module.RemoveMessageLoop();
    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int nCmdShow)
{
    ::InitCommonControls();

    _Module.Init(NULL, hInstance);

    int nRet = Run(lpCmdLine, nCmdShow);

    _Module.Term();

    return nRet;
}
