#include "DataImportExport.h"
#include "db/DeviceRepository.h"
#include "json.hpp"
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

static std::wstring FormatParamsForDisplay(const std::wstring& paramsJson) {
    try {
        json j = json::parse(WStringToString(paramsJson));
        if (!j.is_object()) return paramsJson;
        
        std::wstring result;
        bool first = true;
        
        for (auto& [key, value] : j.items()) {
            if (!first) {
                result += L", ";
            }
            first = false;
            
            result += StringToWString(key) + L":";
            
            if (value.is_string()) {
                result += StringToWString(value.get<std::string>());
            } else if (value.is_number()) {
                result += StringToWString(value.dump());
            } else {
                result += StringToWString(value.dump());
            }
        }
        
        return result;
    } catch (...) {
        return paramsJson;
    }
}

static std::string EscapeCsvField(const std::string& field) {
    bool needsQuotes = (field.find(',') != std::string::npos) || 
                       (field.find('\"') != std::string::npos) || 
                       (field.find('\n') != std::string::npos);
    
    if (!needsQuotes) return field;
    
    std::string result = "\"";
    for (char c : field) {
        if (c == '\"') {
            result += "\"\"";
        } else {
            result += c;
        }
    }
    result += "\"";
    return result;
}

bool DataImportExport::ExportToExcel(const std::wstring& filePath) {
    auto devices = DeviceRepository::Instance().GetAll();
    
    std::string filePathUtf8 = WStringToString(filePath);
    std::ofstream file(filePathUtf8);
    if (!file.is_open()) {
        return false;
    }
    
    file << "ID,设备名称,识别码,状态,设备参数,创建时间,更新时间\n";
    
    for (const auto& device : devices) {
        std::wstring paramsDisplay = FormatParamsForDisplay(device.params);
        
        file << device.id << ",";
        file << EscapeCsvField(WStringToString(device.name)) << ",";
        file << EscapeCsvField(WStringToString(device.code)) << ",";
        file << EscapeCsvField(WStringToString(StatusToString(device.status))) << ",";
        file << EscapeCsvField(WStringToString(paramsDisplay)) << ",";
        file << EscapeCsvField(WStringToString(device.created_at)) << ",";
        file << EscapeCsvField(WStringToString(device.updated_at)) << "\n";
    }
    
    file.close();
    return true;
}

ImportResult DataImportExport::ImportFromExcel(const std::wstring& filePath) {
    ImportResult result;
    
    std::wstring csvPath = filePath;
    size_t pos = csvPath.find_last_of(L'.');
    if (pos != std::wstring::npos) {
        csvPath = csvPath.substr(0, pos) + L".csv";
    }
    
    std::string filePathUtf8 = WStringToString(filePath);
    std::ifstream file(filePathUtf8);
    if (!file.is_open()) {
        return result;
    }
    
    std::string line;
    bool isHeader = true;
    int nameCol = -1;
    int codeCol = -1;
    int statusCol = -1;
    int paramsCol = -1;
    
    std::vector<Device> devices;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::vector<std::string> columns;
        std::stringstream ss(line);
        std::string col;
        
        while (std::getline(ss, col, ',')) {
            if (!col.empty() && col.front() == '"' && col.back() == '"') {
                col = col.substr(1, col.length() - 2);
            }
            columns.push_back(col);
        }
        
        if (isHeader) {
            for (size_t i = 0; i < columns.size(); ++i) {
                if (columns[i] == "设备名称" || columns[i] == "Name") nameCol = (int)i;
                else if (columns[i] == "识别码" || columns[i] == "Code") codeCol = (int)i;
                else if (columns[i] == "状态" || columns[i] == "Status") statusCol = (int)i;
                else if (columns[i] == "设备参数" || columns[i] == "Params") paramsCol = (int)i;
            }
            isHeader = false;
            
            if (nameCol == -1) {
                file.close();
                return result;
            }
            continue;
        }
        
        if (nameCol >= (int)columns.size()) {
            result.failed++;
            continue;
        }
        
        Device device;
        device.name = StringToWString(columns[nameCol]);
        
        if (device.name.empty()) {
            result.failed++;
            continue;
        }
        
        if (codeCol != -1 && codeCol < (int)columns.size()) {
            device.code = StringToWString(columns[codeCol]);
        }
        
        if (statusCol != -1 && statusCol < (int)columns.size()) {
            device.status = StringToStatus(StringToWString(columns[statusCol]));
        } else {
            device.status = DeviceStatus::IN_USE;
        }
        
        if (paramsCol != -1 && paramsCol < (int)columns.size()) {
            device.params = StringToWString(columns[paramsCol]);
        }
        if (device.params.empty()) {
            device.params = L"{}";
        }
        
        if (!device.code.empty() && DeviceRepository::Instance().ExistsByCode(device.code)) {
            result.skipped++;
            continue;
        }
        
        devices.push_back(device);
    }
    
    file.close();
    
    for (auto& device : devices) {
        if (DeviceRepository::Instance().Add(device)) {
            result.success++;
        } else {
            result.failed++;
        }
    }
    
    return result;
}
