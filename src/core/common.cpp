#include "common.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

std::wstring StatusToString(DeviceStatus status) {
    switch (status) {
        case DeviceStatus::IN_USE: return L"使用中";
        case DeviceStatus::SCRAPPED: return L"报废";
        default: return L"未知";
    }
}

DeviceStatus StringToStatus(const std::wstring& str) {
    if (str == L"使用中") return DeviceStatus::IN_USE;
    if (str == L"报废") return DeviceStatus::SCRAPPED;
    return DeviceStatus::IN_USE;
}

std::wstring GenerateUniqueCode() {
    static const wchar_t chars[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 35);

    std::wstring code;
    code.reserve(10);
    
    for (int i = 0; i < 10; ++i) {
        code += chars[dis(gen)];
    }
    return code;
}

std::wstring GetCurrentDateTime() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    wchar_t buf[64];
    swprintf_s(buf, 64, L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
    
    return std::wstring(buf);
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size, NULL, NULL);
    str.pop_back();
    return str;
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    wstr.pop_back();
    return wstr;
}
