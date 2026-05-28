#include "ScanCodeListener.h"
#include <chrono>

ScanCodeListener::ScanCodeListener() : running_(false) {}

ScanCodeListener::~ScanCodeListener() {
    Stop();
}

ScanCodeListener& ScanCodeListener::Instance() {
    static ScanCodeListener instance;
    return instance;
}

bool ScanCodeListener::Start(HWND hTargetWnd) {
    if (running_.load()) return false;
    
    targetWnd_ = hTargetWnd;
    running_.store(true);
    buffer_.clear();
    
    listenThread_ = std::thread(&ScanCodeListener::ListenThread, this);
    return true;
}

void ScanCodeListener::Stop() {
    running_.store(false);
    if (listenThread_.joinable()) {
        listenThread_.join();
    }
    targetWnd_ = nullptr;
    buffer_.clear();
}

void ScanCodeListener::ListenThread() {
    DWORD lastInputTime = 0;
    std::wstring currentCode;
    
    while (running_.load()) {
        for (int vk = 0; vk < 256; ++vk) {
            if (GetAsyncKeyState(vk) & 0x8000) {
                DWORD currentTime = GetTickCount();
                
                if (currentTime - lastInputTime > 50 && !currentCode.empty()) {
                    if (IsValidScanCode(currentCode)) {
                        std::wstring* pCode = new std::wstring(currentCode);
                        PostMessage(targetWnd_, WM_USER_SCAN_CODE, 0, (LPARAM)pCode);
                    }
                    currentCode.clear();
                }
                
                lastInputTime = currentTime;
                
                if (vk >= '0' && vk <= '9') {
                    currentCode += (wchar_t)vk;
                } else if (vk >= 'A' && vk <= 'Z') {
                    if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
                        currentCode += (wchar_t)(vk + 32);
                    } else {
                        currentCode += (wchar_t)vk;
                    }
                } else if (vk == VK_RETURN) {
                    if (IsValidScanCode(currentCode)) {
                        std::wstring* pCode = new std::wstring(currentCode);
                        PostMessage(targetWnd_, WM_USER_SCAN_CODE, 0, (LPARAM)pCode);
                    }
                    currentCode.clear();
                } else if (vk == VK_BACK) {
                    if (!currentCode.empty()) {
                        currentCode.pop_back();
                    }
                }
                
                while (GetAsyncKeyState(vk) & 0x8000) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    if (!running_.load()) break;
                }
            }
            if (!running_.load()) break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool ScanCodeListener::IsValidScanCode(const std::wstring& code) {
    if (code.empty()) return false;
    if (code.length() < minCodeLength_ || code.length() > maxCodeLength_) return false;
    
    for (wchar_t c : code) {
        if (!iswalnum(c)) return false;
    }
    
    if (!prefix_.empty() && code.substr(0, prefix_.length()) != prefix_) return false;
    
    return true;
}

std::wstring ScanCodeListener::StripSuffix(const std::wstring& code) {
    if (!suffix_.empty() && code.length() > suffix_.length() && 
        code.substr(code.length() - suffix_.length()) == suffix_) {
        return code.substr(0, code.length() - suffix_.length());
    }
    
    return code;
}
