#pragma once

#include "core/common.h"
#include "zint.h"
#include <vector>
#include <memory>

struct BarcodeImage {
    int width = 0;
    int height = 0;
    std::vector<BYTE> data;
    
    HBITMAP ToHBITMAP(HDC hdc) const;
};

class BarcodeGenerator {
public:
    static BarcodeGenerator& Instance();
    
    std::optional<BarcodeImage> GenerateCode128(const std::wstring& code, int width = 400, int height = 150);
    
private:
    BarcodeGenerator();
    ~BarcodeGenerator();
    BarcodeGenerator(const BarcodeGenerator&) = delete;
    BarcodeGenerator& operator=(const BarcodeGenerator&) = delete;
};
