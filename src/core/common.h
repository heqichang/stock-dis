#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06010000
#endif

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <ctime>
#include <cstdint>

#define WM_USER_SCAN_CODE (WM_USER + 100)
#define WM_USER_REFRESH_LIST (WM_USER + 101)

enum class DeviceStatus {
    IN_USE = 0,
    SCRAPPED = 1
};

struct Device {
    int64_t id = 0;
    std::wstring name;
    std::wstring code;
    DeviceStatus status = DeviceStatus::IN_USE;
    std::wstring params;
    std::wstring created_at;
    std::wstring updated_at;
};

struct DeviceQuery {
    std::wstring keyword;
    DeviceStatus status = (DeviceStatus)-1;
    int page = 1;
    int page_size = 20;
};

struct PagedResult {
    std::vector<Device> items;
    int total = 0;
    int page = 1;
    int page_size = 20;
    int page_count = 0;
};

enum class ParamFieldType {
    TEXT = 0,
    NUMBER = 1,
    DATE = 2,
    SELECT = 3
};

struct ParamField {
    int64_t id = 0;
    std::wstring field_key;
    std::wstring field_label;
    ParamFieldType field_type = ParamFieldType::TEXT;
    std::wstring field_options;
    int sort_order = 0;
    bool is_required = false;
    std::wstring created_at;
};

std::wstring StatusToString(DeviceStatus status);
ParamFieldType StringToFieldType(const std::wstring& str);
std::wstring FieldTypeToString(ParamFieldType type);
DeviceStatus StringToStatus(const std::wstring& str);
std::wstring GenerateUniqueCode();
std::wstring GetCurrentDateTime();
std::string WStringToString(const std::wstring& wstr);
std::wstring StringToWString(const std::string& str);
