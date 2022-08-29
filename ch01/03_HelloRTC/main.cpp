#include "atl.h"
#include "atldlgs.h"
#include "resource.h"
#include "maindlg.h"
#include "Net/DNet.h"
#include <memory>

CAppModule _Module;

int Run(LPTSTR /*lpCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    std::shared_ptr<CMainDlg> pDlg(new CMainDlg);

    if (pDlg->Create(NULL) == NULL)
    {
        ATLTRACE(_T("Main dialog creation failed!\n"));
        return 0;
    }

    pDlg->ShowWindow(nCmdShow);

    int nRet = theLoop.Run();

    _Module.RemoveMessageLoop();
    return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int nCmdShow)
{
    ::InitCommonControls();
    DNet::Init();

    _Module.Init(NULL, hInstance);

    int nRet = Run(lpCmdLine, nCmdShow);

    _Module.Term();

    DNet::UnInit();
    return nRet;
}
