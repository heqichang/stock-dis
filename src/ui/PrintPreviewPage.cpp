#include "PrintPreviewPage.h"
#include "ui/UIHelper.h"
#include <sstream>

PrintPreviewPage::PrintPreviewPage() {}

void PrintPreviewPage::OnCreate() {
    controls_[ID_BACK_BUTTON] = UIHelper::CreateButton(hWnd_, ID_BACK_BUTTON, L"返回", 20, 15, 80, 30);
    controls_[ID_PRINT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_PRINT_BUTTON, L"打印", 110, 15, 80, 30);
    controls_[ID_ZOOM_IN] = UIHelper::CreateButton(hWnd_, ID_ZOOM_IN, L"放大", 200, 15, 60, 30);
    controls_[ID_ZOOM_OUT] = UIHelper::CreateButton(hWnd_, ID_ZOOM_OUT, L"缩小", 270, 15, 60, 30);
    controls_[ID_PREV_PAGE] = UIHelper::CreateButton(hWnd_, ID_PREV_PAGE, L"上一页", 340, 15, 80, 30);
    controls_[ID_NEXT_PAGE] = UIHelper::CreateButton(hWnd_, ID_NEXT_PAGE, L"下一页", 430, 15, 80, 30);
}

void PrintPreviewPage::OnPaint(HDC hdc) {
    RECT rcClient;
    GetClientRect(hWnd_, &rcClient);
    
    RECT previewRect = {20, 60, rcClient.right - 20, rcClient.bottom - 20};
    FillRect(hdc, &previewRect, (HBRUSH)GetStockObject(GRAY_BRUSH));
    
    int paperWidth = (int)(labelWidth_ * cols_ * zoom_) + 40;
    int paperHeight = (int)(labelHeight_ * rows_ * zoom_) + 40;
    int paperX = previewRect.left + (previewRect.right - previewRect.left - paperWidth) / 2;
    int paperY = previewRect.top + (previewRect.bottom - previewRect.top - paperHeight) / 2;
    
    RECT paperRect = {paperX, paperY, paperX + paperWidth, paperY + paperHeight};
    FillRect(hdc, &paperRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    Rectangle(hdc, paperX, paperY, paperX + paperWidth, paperY + paperHeight);
    
    int startIdx = currentPage_ * labelsPerPage_;
    int endIdx = min(startIdx + labelsPerPage_, (int)labels_.size());
    
    for (int i = startIdx; i < endIdx; ++i) {
        int idx = i - startIdx;
        int col = idx % cols_;
        int row = idx / cols_;
        
        int x = paperX + 20 + col * (int)(labelWidth_ * zoom_);
        int y = paperY + 20 + row * (int)(labelHeight_ * zoom_);
        int w = (int)(labelWidth_ * zoom_);
        int h = (int)(labelHeight_ * zoom_);
        
        HDC hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hMemBitmap = CreateCompatibleBitmap(hdc, labelWidth_, labelHeight_);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hMemBitmap);
        
        RECT memRect = {0, 0, labelWidth_, labelHeight_};
        FillRect(hMemDC, &memRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        PrintService::Instance().DrawLabel(hMemDC, labels_[i], 0, 0, labelWidth_, labelHeight_);
        
        SetStretchBltMode(hdc, HALFTONE);
        StretchBlt(hdc, x, y, w, h, hMemDC, 0, 0, labelWidth_, labelHeight_, SRCCOPY);
        
        SelectObject(hMemDC, hOldBitmap);
        DeleteObject(hMemBitmap);
        DeleteDC(hMemDC);
    }
    
    HFONT hInfoFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hInfoFont);
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    
    std::wstringstream ss;
    ss << L"第 " << currentPage_ + 1 << L" / " << totalPages_ << L" 页，共 " << labels_.size() << L" 个标签，缩放: " << (int)(zoom_ * 100) << L"%";
    RECT infoRect = {20, 25, rcClient.right - 20, 55};
    DrawText(hdc, ss.str().c_str(), -1, &infoRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hInfoFont);
}

void PrintPreviewPage::OnSize(int cx, int cy) {
    if (cx < 400 || cy < 300) return;
    
    CalculateLayout();
    InvalidateRect(hWnd_, nullptr, TRUE);
}

void PrintPreviewPage::OnCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
        case ID_BACK_BUTTON:
            OnBack();
            break;
        case ID_PRINT_BUTTON:
            OnPrint();
            break;
        case ID_ZOOM_IN:
            OnZoomIn();
            break;
        case ID_ZOOM_OUT:
            OnZoomOut();
            break;
        case ID_PREV_PAGE:
            OnPrevPage();
            break;
        case ID_NEXT_PAGE:
            OnNextPage();
            break;
    }
}

void PrintPreviewPage::SetLabels(const std::vector<PrintLabel>& labels) {
    labels_ = labels;
    currentPage_ = 0;
    zoom_ = 1.0f;
    
    if (hWnd_) {
        HDC hdc = GetDC(hWnd_);
        labelWidth_ = (int)(50.0f * GetDeviceCaps(hdc, LOGPIXELSX) / 25.4f);
        labelHeight_ = (int)(30.0f * GetDeviceCaps(hdc, LOGPIXELSY) / 25.4f);
        ReleaseDC(hWnd_, hdc);
        
        CalculateLayout();
        InvalidateRect(hWnd_, nullptr, TRUE);
        UpdateWindow(hWnd_);
    }
}

void PrintPreviewPage::CalculateLayout() {
    if (labels_.empty()) {
        totalPages_ = 0;
        labelsPerPage_ = 0;
        return;
    }
    
    RECT rcClient;
    GetClientRect(hWnd_, &rcClient);
    
    int availableWidth = rcClient.right - rcClient.left - 80;
    int availableHeight = rcClient.bottom - rcClient.top - 120;
    
    cols_ = max(1, availableWidth / (int)(labelWidth_ * zoom_));
    rows_ = max(1, availableHeight / (int)(labelHeight_ * zoom_));
    
    labelsPerPage_ = cols_ * rows_;
    totalPages_ = (labels_.size() + labelsPerPage_ - 1) / labelsPerPage_;
    
    if (currentPage_ >= totalPages_) {
        currentPage_ = totalPages_ - 1;
    }
    
    EnableWindow(controls_[ID_PREV_PAGE], currentPage_ > 0);
    EnableWindow(controls_[ID_NEXT_PAGE], currentPage_ < totalPages_ - 1);
}

void PrintPreviewPage::OnBack() {
    SendMessage(hParent_, WM_COMMAND, 13, 0);
}

void PrintPreviewPage::OnPrint() {
    PrintService::Instance().PrintLabels(labels_);
}

void PrintPreviewPage::OnZoomIn() {
    zoom_ = min(zoom_ * 1.25f, 3.0f);
    CalculateLayout();
    InvalidateRect(hWnd_, nullptr, TRUE);
}

void PrintPreviewPage::OnZoomOut() {
    zoom_ = max(zoom_ * 0.8f, 0.25f);
    CalculateLayout();
    InvalidateRect(hWnd_, nullptr, TRUE);
}

void PrintPreviewPage::OnPrevPage() {
    if (currentPage_ > 0) {
        currentPage_--;
        InvalidateRect(hWnd_, nullptr, TRUE);
        EnableWindow(controls_[ID_PREV_PAGE], currentPage_ > 0);
        EnableWindow(controls_[ID_NEXT_PAGE], currentPage_ < totalPages_ - 1);
    }
}

void PrintPreviewPage::OnNextPage() {
    if (currentPage_ < totalPages_ - 1) {
        currentPage_++;
        InvalidateRect(hWnd_, nullptr, TRUE);
        EnableWindow(controls_[ID_PREV_PAGE], currentPage_ > 0);
        EnableWindow(controls_[ID_NEXT_PAGE], currentPage_ < totalPages_ - 1);
    }
}
