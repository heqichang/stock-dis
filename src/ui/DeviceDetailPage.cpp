#include "DeviceDetailPage.h"
#include "ui/UIHelper.h"
#include "db/DeviceRepository.h"
#include "print/PrintService.h"
#include <sstream>

DeviceDetailPage::DeviceDetailPage() {}

void DeviceDetailPage::OnCreate() {
    controls_[ID_BACK_BUTTON] = UIHelper::CreateButton(hWnd_, ID_BACK_BUTTON, L"返回列表", 20, 20, 100, 30);
    controls_[ID_EDIT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_EDIT_BUTTON, L"编辑", 130, 20, 80, 30);
    controls_[ID_PRINT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_PRINT_BUTTON, L"打印标签", 220, 20, 100, 30);
    controls_[ID_SCRAP_BUTTON] = UIHelper::CreateButton(hWnd_, ID_SCRAP_BUTTON, L"报废设备", 330, 20, 100, 30);
    
    if (deviceId_ > 0) {
        LoadDevice();
    }
}

void DeviceDetailPage::OnPaint(HDC hdc) {
    RECT rcClient;
    GetClientRect(hWnd_, &rcClient);
    
    HFONT hTitleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hLabelFont = CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hValueFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hCodeFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");
    
    if (device_) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
        SetTextColor(hdc, RGB(0, 102, 204));
        RECT titleRect = {30, 70, rcClient.right - 30, 110};
        DrawText(hdc, device_->name.c_str(), -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetTextColor(hdc, RGB(0, 0, 0));
        SelectObject(hdc, hLabelFont);
        
        int y = 120;
        int labelX = 30;
        int valueX = 130;
        int lineHeight = 35;
        
        RECT labelRect, valueRect;
        
        SetRect(&labelRect, labelX, y, valueX - 10, y + lineHeight);
        DrawText(hdc, L"识别码:", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetRect(&valueRect, valueX, y, rcClient.right - 30, y + lineHeight);
        SelectObject(hdc, hCodeFont);
        SetTextColor(hdc, RGB(51, 51, 51));
        DrawText(hdc, device_->code.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SetTextColor(hdc, RGB(0, 0, 0));
        SelectObject(hdc, hLabelFont);
        
        y += lineHeight;
        SetRect(&labelRect, labelX, y, valueX - 10, y + lineHeight);
        DrawText(hdc, L"状态:", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetRect(&valueRect, valueX, y, rcClient.right - 30, y + lineHeight);
        SelectObject(hdc, hValueFont);
        if (device_->status == DeviceStatus::IN_USE) {
            SetTextColor(hdc, RGB(0, 153, 0));
        } else {
            SetTextColor(hdc, RGB(204, 0, 0));
        }
        DrawText(hdc, StatusToString(device_->status).c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SetTextColor(hdc, RGB(0, 0, 0));
        SelectObject(hdc, hLabelFont);
        
        y += lineHeight;
        SetRect(&labelRect, labelX, y, valueX - 10, y + lineHeight);
        DrawText(hdc, L"创建时间:", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetRect(&valueRect, valueX, y, rcClient.right - 30, y + lineHeight);
        SelectObject(hdc, hValueFont);
        DrawText(hdc, device_->created_at.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hLabelFont);
        
        y += lineHeight;
        SetRect(&labelRect, labelX, y, valueX - 10, y + lineHeight);
        DrawText(hdc, L"更新时间:", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetRect(&valueRect, valueX, y, rcClient.right - 30, y + lineHeight);
        SelectObject(hdc, hValueFont);
        DrawText(hdc, device_->updated_at.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hLabelFont);
        
        y += lineHeight + 10;
        SetRect(&labelRect, labelX, y, rcClient.right - 30, y + lineHeight);
        DrawText(hdc, L"设备参数:", -1, &labelRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
        
        y += lineHeight;
        RECT paramsRect = {labelX, y, rcClient.right - 30, y + 120};
        SelectObject(hdc, hValueFont);
        DrawText(hdc, device_->params.c_str(), -1, &paramsRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EDITCONTROL);
        
        DrawBarcode(hdc);
        
        SelectObject(hdc, hOldFont);
    } else {
        HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
        RECT rect = {30, 100, rcClient.right - 30, 200};
        DrawText(hdc, L"设备不存在或已被删除", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hOldFont);
    }
    
    DeleteObject(hTitleFont);
    DeleteObject(hLabelFont);
    DeleteObject(hValueFont);
    DeleteObject(hCodeFont);
}

void DeviceDetailPage::DrawBarcode(HDC hdc) {
    if (!barcode_ || barcode_->width <= 0 || barcode_->height <= 0) return;
    
    RECT rcClient;
    GetClientRect(hWnd_, &rcClient);
    
    int barcodeWidth = min(barcode_->width * 2, rcClient.right - 300);
    int barcodeHeight = min(barcode_->height * 2, 200);
    int x = rcClient.right - barcodeWidth - 40;
    int y = 120;
    
    RECT barcodeRect = {x - 10, y - 10, x + barcodeWidth + 10, y + barcodeHeight + 60};
    FillRect(hdc, &barcodeRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    FrameRect(hdc, &barcodeRect, (HBRUSH)GetStockObject(GRAY_BRUSH));
    
    HBITMAP hBitmap = barcode_->ToHBITMAP(hdc);
    if (hBitmap) {
        HDC hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
        
        SetStretchBltMode(hdc, HALFTONE);
        StretchBlt(hdc, x, y, barcodeWidth, barcodeHeight,
                  hMemDC, 0, 0, barcode_->width, barcode_->height, SRCCOPY);
        
        SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
    }
    
    HFONT hNameFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hCodeFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");
    
    RECT nameRect = {x, y + barcodeHeight + 5, x + barcodeWidth, y + barcodeHeight + 30};
    HFONT hOldFont = (HFONT)SelectObject(hdc, hNameFont);
    DrawText(hdc, device_->name.c_str(), -1, &nameRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    
    RECT codeRect = {x, y + barcodeHeight + 30, x + barcodeWidth, y + barcodeHeight + 55};
    SelectObject(hdc, hCodeFont);
    DrawText(hdc, device_->code.c_str(), -1, &codeRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hNameFont);
    DeleteObject(hCodeFont);
}

void DeviceDetailPage::OnSize(int cx, int cy) {
    if (cx < 400 || cy < 300) return;
    
    InvalidateRect(hWnd_, nullptr, TRUE);
}

void DeviceDetailPage::OnCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
        case ID_BACK_BUTTON:
            OnBack();
            break;
        case ID_EDIT_BUTTON:
            OnEdit();
            break;
        case ID_PRINT_BUTTON:
            OnPrint();
            break;
        case ID_SCRAP_BUTTON:
            OnScrap();
            break;
    }
}

void DeviceDetailPage::SetDevice(int64_t deviceId) {
    deviceId_ = deviceId;
    if (hWnd_) {
        LoadDevice();
    }
}

void DeviceDetailPage::LoadDevice() {
    device_ = DeviceRepository::Instance().GetById(deviceId_);
    if (device_) {
        barcode_ = BarcodeGenerator::Instance().GenerateCode128(device_->code);
        
        EnableWindow(controls_[ID_SCRAP_BUTTON], device_->status == DeviceStatus::IN_USE);
        if (device_->status == DeviceStatus::SCRAPPED) {
            SetWindowText(controls_[ID_SCRAP_BUTTON], L"已报废");
        } else {
            SetWindowText(controls_[ID_SCRAP_BUTTON], L"报废设备");
        }
    }
    InvalidateRect(hWnd_, nullptr, TRUE);
}

void DeviceDetailPage::OnBack() {
    SendMessage(hParent_, WM_COMMAND, 12, 0);
}

void DeviceDetailPage::OnEdit() {
    SendMessage(hParent_, WM_COMMAND, 2, (LPARAM)deviceId_);
}

void DeviceDetailPage::OnPrint() {
    if (!device_) return;
    
    PrintLabel label;
    label.name = device_->name;
    label.code = device_->code;
    label.barcode = barcode_;
    
    std::vector<PrintLabel> labels;
    labels.push_back(label);
    
    PrintService::Instance().PrintLabels(labels);
}

void DeviceDetailPage::OnScrap() {
    if (!device_ || device_->status != DeviceStatus::IN_USE) return;
    
    if (!UIHelper::ShowConfirmDialog(hWnd_, L"确定要将该设备报废吗？此操作不可逆！", L"确认报废")) {
        return;
    }
    
    if (DeviceRepository::Instance().UpdateStatus(deviceId_, DeviceStatus::SCRAPPED)) {
        UIHelper::ShowMessageBox(hWnd_, L"设备已报废", L"提示");
        LoadDevice();
    } else {
        UIHelper::ShowMessageBox(hWnd_, L"操作失败", L"错误", MB_OK | MB_ICONERROR);
    }
}
