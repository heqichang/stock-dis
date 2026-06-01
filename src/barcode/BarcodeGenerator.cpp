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
    if (code_utf8.empty()) return std::nullopt;

    zint_symbol* symbol = ZBarcode_Create();
    if (!symbol) return std::nullopt;

    symbol->symbology = BARCODE_CODE128;
    symbol->show_hrt = 0;
    symbol->border_width = 0;
    symbol->output_options = 0;
    symbol->input_mode = UNICODE_MODE;

    int ret = ZBarcode_Encode(symbol, reinterpret_cast<const unsigned char*>(code_utf8.c_str()), (int)code_utf8.length());
    if (ret != 0) {
        ZBarcode_Delete(symbol);
        return std::nullopt;
    }

    int codeModules = symbol->width;
    if (codeModules <= 0) {
        ZBarcode_Delete(symbol);
        return std::nullopt;
    }

    int quietZone = 10;
    symbol->whitespace_width = quietZone;
    symbol->whitespace_height = 0;

    int totalModules = codeModules + 2 * quietZone;
    int targetScale = width / totalModules;
    if (targetScale < 1) targetScale = 1;
    if (targetScale > 4) targetScale = 4;

    symbol->scale = (float)targetScale;
    symbol->height = (float)(height * 0.6f);
    if (symbol->height < 10.0f) symbol->height = 10.0f;

    ret = ZBarcode_Buffer(symbol, 0);
    if (ret != 0 || !symbol->bitmap || symbol->bitmap_width <= 0 || symbol->bitmap_height <= 0) {
        ZBarcode_Delete(symbol);
        return std::nullopt;
    }

    BarcodeImage image;
    image.width = symbol->bitmap_width;
    image.height = symbol->bitmap_height;
    int bitmap_size = image.width * image.height * 3;
    image.data.resize(bitmap_size);
    memcpy(image.data.data(), symbol->bitmap, bitmap_size);

    ZBarcode_Delete(symbol);
    return image;
}
