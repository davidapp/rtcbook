#include "atl.h"
#include "atldlgs.h"
#include "resource.h"
#include "SettingDlg.h"
#include "Video/WinDSCamera.h"

CAppModule _Module;

int Run(LPTSTR /*lpCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CSettingDlg dlg;
    dlg.DoModal();

    int nRet = theLoop.Run();

    _Module.RemoveMessageLoop();
    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int nCmdShow)
{
    ::InitCommonControls();

    _Module.Init(NULL, hInstance);
    WinDSCamera::Init();

    int nRet = Run(lpCmdLine, nCmdShow);

    WinDSCamera::UnInit();
    _Module.Term();

    return nRet;
}
