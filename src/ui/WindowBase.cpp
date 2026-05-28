#include "WindowBase.h"

static const wchar_t* WINDOW_CLASS_NAME = L"DeviceManagerWindowClass";

WindowBase::WindowBase() {}

WindowBase::~WindowBase() {
    Destroy();
}

bool WindowBase::Create(HWND hParent, const std::wstring& title, int x, int y, int width, int height) {
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = WINDOW_CLASS_NAME;
    
    RegisterClassEx(&wcex);
    
    hParent_ = hParent;
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exStyle = 0;
    
    if (hParent) {
        style = WS_CHILD | WS_VISIBLE;
        exStyle = 0;
    }
    
    hWnd_ = CreateWindowEx(exStyle, WINDOW_CLASS_NAME, title.c_str(), style,
                          x, y, width, height, hParent, nullptr, GetModuleHandle(nullptr), this);
    
    if (!hWnd_) return false;
    
    return true;
}

void WindowBase::Destroy() {
    if (hWnd_) {
        DestroyWindow(hWnd_);
        hWnd_ = nullptr;
    }
}

void WindowBase::Show(int nCmdShow) {
    if (hWnd_) {
        ShowWindow(hWnd_, nCmdShow);
        UpdateWindow(hWnd_);
    }
}

void WindowBase::Hide() {
    if (hWnd_) {
        ShowWindow(hWnd_, SW_HIDE);
    }
}

void WindowBase::CenterWindow() {
    if (!hWnd_ || !hParent_) return;
    
    RECT rcParent, rcWindow;
    GetWindowRect(hParent_, &rcParent);
    GetWindowRect(hWnd_, &rcWindow);
    
    int x = rcParent.left + (rcParent.right - rcParent.left - (rcWindow.right - rcWindow.left)) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - (rcWindow.bottom - rcWindow.top)) / 2;
    
    SetWindowPos(hWnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

LRESULT CALLBACK WindowBase::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WindowBase* pThis = nullptr;
    
    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<WindowBase*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->hWnd_ = hWnd;
        pThis->OnCreate();
    } else {
        pThis = reinterpret_cast<WindowBase*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    
    if (pThis) {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT WindowBase::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            OnDestroy();
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hWnd_, &ps);
            OnPaint();
            EndPaint(hWnd_, &ps);
            break;
        }
            
        case WM_SIZE:
            OnSize(LOWORD(lParam), HIWORD(lParam));
            break;
            
        case WM_COMMAND:
            OnCommand(wParam, lParam);
            break;
            
        default:
            return DefWindowProc(hWnd_, uMsg, wParam, lParam);
    }
    
    return 0;
}
