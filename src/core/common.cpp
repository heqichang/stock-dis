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

ParamFieldType StringToFieldType(const std::wstring& str) {
    if (str == L"number") return ParamFieldType::NUMBER;
    if (str == L"date") return ParamFieldType::DATE;
    if (str == L"select") return ParamFieldType::SELECT;
    return ParamFieldType::TEXT;
}

std::wstring FieldTypeToString(ParamFieldType type) {
    switch (type) {
        case ParamFieldType::TEXT: return L"text";
        case ParamFieldType::NUMBER: return L"number";
        case ParamFieldType::DATE: return L"date";
        case ParamFieldType::SELECT: return L"select";
        default: return L"text";
    }
}

std::wstring GenerateUniqueCode() {
    extern int64_t GetNextCodeSequenceFromRepo();
    int64_t sequence = GetNextCodeSequenceFromRepo();
    if (sequence == 0) {
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
    
    wchar_t buf[32];
    swprintf_s(buf, 32, L"TSG%08I64d", sequence);
    return std::wstring(buf);
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
