#include "DataImportExport.h"
#include "db/DeviceRepository.h"
#include "nlohmann/json.hpp"
#include "xlsxwriter.h"
#include <fstream>
#include <sstream>

using json = nlohmann::json;

DataImportExport::DataImportExport() {}

DataImportExport::~DataImportExport() {}

DataImportExport& DataImportExport::Instance() {
    static DataImportExport instance;
    return instance;
}

bool DataImportExport::ExportToJson(const std::wstring& filePath) {
    auto devices = DeviceRepository::Instance().GetAll();
    
    json j;
    for (const auto& device : devices) {
        json jDevice;
        jDevice["id"] = device.id;
        jDevice["name"] = WStringToString(device.name);
        jDevice["code"] = WStringToString(device.code);
        jDevice["status"] = WStringToString(StatusToString(device.status));
        jDevice["params"] = WStringToString(device.params);
        jDevice["created_at"] = WStringToString(device.created_at);
        jDevice["updated_at"] = WStringToString(device.updated_at);
        j.push_back(jDevice);
    }
    
    std::string filePathUtf8 = WStringToString(filePath);
    std::ofstream file(filePathUtf8);
    if (!file.is_open()) return false;
    
    file << j.dump(4);
    file.close();
    return true;
}

bool DataImportExport::ExportToExcel(const std::wstring& filePath) {
    auto devices = DeviceRepository::Instance().GetAll();
    
    std::string filePathUtf8 = WStringToString(filePath);
    lxw_workbook* workbook = workbook_new(filePathUtf8.c_str());
    if (!workbook) return false;
    
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, "设备列表");
    
    lxw_format* header_format = workbook_add_format(workbook);
    format_set_bold(header_format);
    format_set_bg_color(header_format, LXW_COLOR_BLUE);
    format_set_font_color(header_format, LXW_COLOR_WHITE);
    
    worksheet_write_string(worksheet, 0, 0, "ID", header_format);
    worksheet_write_string(worksheet, 0, 1, "设备名称", header_format);
    worksheet_write_string(worksheet, 0, 2, "识别码", header_format);
    worksheet_write_string(worksheet, 0, 3, "状态", header_format);
    worksheet_write_string(worksheet, 0, 4, "设备参数", header_format);
    worksheet_write_string(worksheet, 0, 5, "创建时间", header_format);
    worksheet_write_string(worksheet, 0, 6, "更新时间", header_format);
    
    for (size_t i = 0; i < devices.size(); ++i) {
        int row = (int)(i + 1);
        const auto& device = devices[i];
        
        worksheet_write_number(worksheet, row, 0, device.id, nullptr);
        worksheet_write_string(worksheet, row, 1, WStringToString(device.name).c_str(), nullptr);
        worksheet_write_string(worksheet, row, 2, WStringToString(device.code).c_str(), nullptr);
        worksheet_write_string(worksheet, row, 3, WStringToString(StatusToString(device.status)).c_str(), nullptr);
        worksheet_write_string(worksheet, row, 4, WStringToString(device.params).c_str(), nullptr);
        worksheet_write_string(worksheet, row, 5, WStringToString(device.created_at).c_str(), nullptr);
        worksheet_write_string(worksheet, row, 6, WStringToString(device.updated_at).c_str(), nullptr);
    }
    
    worksheet_set_column(worksheet, 0, 0, 8, nullptr);
    worksheet_set_column(worksheet, 1, 1, 30, nullptr);
    worksheet_set_column(worksheet, 2, 2, 20, nullptr);
    worksheet_set_column(worksheet, 3, 3, 10, nullptr);
    worksheet_set_column(worksheet, 4, 4, 40, nullptr);
    worksheet_set_column(worksheet, 5, 5, 20, nullptr);
    worksheet_set_column(worksheet, 6, 6, 20, nullptr);
    
    lxw_error error = workbook_close(workbook);
    return error == LXW_NO_ERROR;
}

bool DataImportExport::ImportFromJson(const std::wstring& filePath) {
    std::string filePathUtf8 = WStringToString(filePath);
    std::ifstream file(filePathUtf8);
    if (!file.is_open()) return false;
    
    json j;
    try {
        file >> j;
    } catch (...) {
        file.close();
        return false;
    }
    file.close();
    
    if (!j.is_array()) return false;
    
    std::vector<Device> devices;
    for (const auto& jDevice : j) {
        Device device;
        device.name = StringToWString(jDevice.value("name", ""));
        device.code = StringToWString(jDevice.value("code", ""));
        device.status = StringToStatus(StringToWString(jDevice.value("status", "使用中")));
        device.params = StringToWString(jDevice.value("params", "{}"));
        
        if (!device.name.empty() && !device.code.empty()) {
            devices.push_back(device);
        }
    }
    
    return DeviceRepository::Instance().Import(devices);
}

bool DataImportExport::ImportFromExcel(const std::wstring& filePath) {
    return false;
}
