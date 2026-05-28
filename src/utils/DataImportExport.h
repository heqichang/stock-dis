#pragma once

#include "core/common.h"
#include <vector>
#include <string>

class DataImportExport {
public:
    static DataImportExport& Instance();
    
    bool ExportToJson(const std::wstring& filePath);
    bool ExportToExcel(const std::wstring& filePath);
    bool ImportFromJson(const std::wstring& filePath);
    bool ImportFromExcel(const std::wstring& filePath);
    
private:
    DataImportExport();
    ~DataImportExport();
    DataImportExport(const DataImportExport&) = delete;
    DataImportExport& operator=(const DataImportExport&) = delete;
};
