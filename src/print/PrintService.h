#pragma once

#include "core/common.h"
#include "barcode/BarcodeGenerator.h"
#include <vector>

struct PrintLabel {
    std::wstring name;
    std::wstring code;
    std::optional<BarcodeImage> barcode;
};

class PrintService {
public:
    static PrintService& Instance();
    
    bool PrintLabel(const PrintLabel& label);
    bool PrintLabels(const std::vector<PrintLabel>& labels);
    void DrawLabel(HDC hdc, const PrintLabel& label, int x, int y, int width, int height);
    bool ShowPrintPreview(HWND hParent, const std::vector<PrintLabel>& labels);
    
private:
    PrintService();
    ~PrintService();
    PrintService(const PrintService&) = delete;
    PrintService& operator=(const PrintService&) = delete;
    
    float MMToPixelsX(HDC hdc, float mm);
    float MMToPixelsY(HDC hdc, float mm);
};
