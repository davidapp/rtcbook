#include "atl.h"
#include "atldlgs.h"
#include "resource.h"
#include "MainDlg.h"

CAppModule _Module;

int Run(LPTSTR /*lpCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CMainDlg dlg;
    dlg.DoModal();

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
