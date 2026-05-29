#include <windows.h>
#include "ui/MainWindow.h"
#include "ui/UIHelper.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    SetProcessDPIAware();
    
    UIHelper::InitCommonControls();
    
    MainWindow mainWnd;
    if (!mainWnd.CreateMainWindow()) {
        return 1;
    }
    
    mainWnd.CenterWindow();
    mainWnd.Show(nCmdShow);
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
