#pragma once

#include "core/common.h"
#include <vector>
#include <string>

struct ImportResult {
    int success = 0;
    int skipped = 0;
    int failed = 0;
};

class DataImportExport {
public:
    static DataImportExport& Instance();
    
    bool ExportToExcel(const std::wstring& filePath);
    ImportResult ImportFromExcel(const std::wstring& filePath);
    
private:
    DataImportExport();
    ~DataImportExport();
    DataImportExport(const DataImportExport&) = delete;
    DataImportExport& operator=(const DataImportExport&) = delete;
};
