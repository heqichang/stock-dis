#pragma once

#include "core/common.h"
#include <string>
#include <map>

class WindowBase {
public:
    WindowBase();
    virtual ~WindowBase();
    
    virtual bool Create(HWND hParent, const std::wstring& title, int x, int y, int width, int height);
    virtual void Destroy();
    HWND GetHandle() const { return hWnd_; }
    
    void Show(int nCmdShow = SW_SHOW);
    void Hide();
    void CenterWindow();
    
protected:
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void OnCreate() {}
    virtual void OnDestroy() {}
    virtual void OnPaint() {}
    virtual void OnSize(int cx, int cy) {}
    virtual void OnCommand(WPARAM wParam, LPARAM lParam) {}
    
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    HWND hWnd_ = nullptr;
    HWND hParent_ = nullptr;
    std::map<int, HWND> controls_;
};
