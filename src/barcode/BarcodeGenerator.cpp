#include "BarcodeGenerator.h"

HBITMAP BarcodeImage::ToHBITMAP(HDC hdc) const {
    if (data.empty() || width <= 0 || height <= 0) return nullptr;
    
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    if (!hBitmap) return nullptr;
    
    int stride = ((width * 3 + 3) / 4) * 4;
    BYTE* dest = static_cast<BYTE*>(pBits);
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int src_idx = (y * width + x) * 3;
            int dest_idx = y * stride + x * 3;
            dest[dest_idx] = data[src_idx + 2];
            dest[dest_idx + 1] = data[src_idx + 1];
            dest[dest_idx + 2] = data[src_idx];
        }
    }
    
    return hBitmap;
}

BarcodeGenerator::BarcodeGenerator() {}

BarcodeGenerator::~BarcodeGenerator() {}

BarcodeGenerator& BarcodeGenerator::Instance() {
    static BarcodeGenerator instance;
    return instance;
}

std::optional<BarcodeImage> BarcodeGenerator::GenerateCode128(const std::wstring& code, int width, int height) {
    std::string code_utf8 = WStringToString(code);
    
    zint_symbol* symbol = ZBarcode_Create();
    if (!symbol) return std::nullopt;
    
    symbol->symbology = BARCODE_CODE128;
    symbol->scale = 2;
    symbol->height = height / symbol->scale;
    symbol->whitespace_width = 10;
    symbol->border_width = 0;
    symbol->output_options = BARCODE_BOX;
    
    int ret = ZBarcode_Encode(symbol, reinterpret_cast<const unsigned char*>(code_utf8.c_str()), 0);
    if (ret != 0) {
        ZBarcode_Delete(symbol);
        return std::nullopt;
    }
    
    int bitmap_width = symbol->bitmap_width;
    int bitmap_height = symbol->bitmap_height;
    
    if (bitmap_width <= 0 || bitmap_height <= 0) {
        ZBarcode_Delete(symbol);
        return std::nullopt;
    }
    
    BarcodeImage image;
    image.width = bitmap_width;
    image.height = bitmap_height;
    image.data.resize(bitmap_width * bitmap_height * 3);
    
    for (int y = 0; y < bitmap_height; ++y) {
        for (int x = 0; x < bitmap_width; ++x) {
            unsigned char pixel = symbol->bitmap[y * bitmap_width + x];
            int idx = (y * bitmap_width + x) * 3;
            image.data[idx] = pixel;
            image.data[idx + 1] = pixel;
            image.data[idx + 2] = pixel;
        }
    }
    
    ZBarcode_Delete(symbol);
    return image;
}
