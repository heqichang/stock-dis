#pragma once

#include "core/common.h"
#include <string>
#include <thread>
#include <atomic>

class ScanCodeListener {
public:
    static ScanCodeListener& Instance();
    
    bool Start(HWND hTargetWnd);
    void Stop();
    bool IsRunning() const { return running_.load(); }
    
    void SetMinCodeLength(int length) { minCodeLength_ = length; }
    void SetMaxCodeLength(int length) { maxCodeLength_ = length; }
    void SetPrefix(const std::wstring& prefix) { prefix_ = prefix; }
    void SetSuffix(const std::wstring& suffix) { suffix_ = suffix; }
    
    std::wstring StripSuffix(const std::wstring& code);
    
private:
    ScanCodeListener();
    ~ScanCodeListener();
    ScanCodeListener(const ScanCodeListener&) = delete;
    ScanCodeListener& operator=(const ScanCodeListener&) = delete;
    
    void ListenThread();
    bool IsValidScanCode(const std::wstring& code);
    
    std::atomic<bool> running_;
    HWND targetWnd_ = nullptr;
    std::thread listenThread_;
    std::wstring buffer_;
    
    int minCodeLength_ = 6;
    int maxCodeLength_ = 20;
    std::wstring prefix_;
    std::wstring suffix_ = L"\r";
};
