#include "PrintService.h"

PrintService::PrintService() {}

PrintService::~PrintService() {}

PrintService& PrintService::Instance() {
    static PrintService instance;
    return instance;
}

float PrintService::MMToPixelsX(HDC hdc, float mm) {
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    return mm * dpi / 25.4f;
}

float PrintService::MMToPixelsY(HDC hdc, float mm) {
    int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    return mm * dpi / 25.4f;
}

void PrintService::DrawLabel(HDC hdc, const PrintLabel& label, int x, int y, int width, int height) {
    RECT rect = {x, y, x + width, y + height};
    
    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    Rectangle(hdc, x, y, x + width, y + height);
    
    int barcode_height = height * 0.55;
    int barcode_width = width * 0.9;
    int barcode_x = x + (width - barcode_width) / 2;
    int barcode_y = y + height * 0.05;
    
    if (label.barcode && label.barcode->width > 0 && label.barcode->height > 0) {
        HBITMAP hBitmap = label.barcode->ToHBITMAP(hdc);
        if (hBitmap) {
            HDC hMemDC = CreateCompatibleDC(hdc);
            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
            
            SetStretchBltMode(hdc, HALFTONE);
            StretchBlt(hdc, barcode_x, barcode_y, barcode_width, barcode_height,
                      hMemDC, 0, 0, label.barcode->width, label.barcode->height, SRCCOPY);
            
            SelectObject(hMemDC, hOldBitmap);
            DeleteDC(hMemDC);
            DeleteObject(hBitmap);
        }
    }
    
    HFONT hNameFont = CreateFont(height * 0.12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hCodeFont = CreateFont(height * 0.1, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");
    
    RECT name_rect = {x + width * 0.05, y + height * 0.68, x + width * 0.95, y + height * 0.82};
    RECT code_rect = {x + width * 0.05, y + height * 0.83, x + width * 0.95, y + height * 0.95};
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hNameFont);
    DrawText(hdc, label.name.c_str(), -1, &name_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    
    SelectObject(hdc, hCodeFont);
    DrawText(hdc, label.code.c_str(), -1, &code_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hNameFont);
    DeleteObject(hCodeFont);
}

bool PrintService::PrintLabel(const PrintLabel& label) {
    std::vector<PrintLabel> labels;
    labels.push_back(label);
    return PrintLabels(labels);
}

bool PrintService::PrintLabels(const std::vector<PrintLabel>& labels) {
    if (labels.empty()) return false;
    
    PRINTDLG pd = {};
    pd.lStructSize = sizeof(PRINTDLG);
    pd.Flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC;
    
    if (!PrintDlg(&pd)) return false;
    
    HDC hdc = pd.hDC;
    if (!hdc) return false;
    
    int label_width_mm = 50;
    int label_height_mm = 30;
    
    int label_width = (int)MMToPixelsX(hdc, label_width_mm);
    int label_height = (int)MMToPixelsY(hdc, label_height_mm);
    
    int page_width = GetDeviceCaps(hdc, PHYSICALWIDTH);
    int page_height = GetDeviceCaps(hdc, PHYSICALHEIGHT);
    int margin_x = GetDeviceCaps(hdc, PHYSICALOFFSETX);
    int margin_y = GetDeviceCaps(hdc, PHYSICALOFFSETY);
    
    int cols = (page_width - 2 * margin_x) / label_width;
    int rows = (page_height - 2 * margin_y) / label_height;
    if (cols < 1) cols = 1;
    if (rows < 1) rows = 1;
    
    int labels_per_page = cols * rows;
    int total_pages = (labels.size() + labels_per_page - 1) / labels_per_page;
    
    DOCINFO di = {};
    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = L"设备标签打印";
    
    if (StartDoc(hdc, &di) <= 0) {
        DeleteDC(hdc);
        return false;
    }
    
    for (int page = 0; page < total_pages; ++page) {
        StartPage(hdc);
        
        int start_idx = page * labels_per_page;
        int end_idx = min(start_idx + labels_per_page, (int)labels.size());
        
        for (int i = start_idx; i < end_idx; ++i) {
            int idx = i - start_idx;
            int col = idx % cols;
            int row = idx / cols;
            
            int x = margin_x + col * label_width;
            int y = margin_y + row * label_height;
            
            DrawLabel(hdc, labels[i], x, y, label_width, label_height);
        }
        
        EndPage(hdc);
    }
    
    EndDoc(hdc);
    DeleteDC(hdc);
    return true;
}
