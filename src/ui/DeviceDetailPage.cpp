#include "DeviceDetailPage.h"
#include "ui/UIHelper.h"
#include "db/DeviceRepository.h"
#include "print/PrintService.h"
#include "json.hpp"
#include <sstream>

using json = nlohmann::json;

DeviceDetailPage::DeviceDetailPage() {}

void DeviceDetailPage::OnCreate() {
    controls_[ID_BACK_BUTTON] = UIHelper::CreateButton(hWnd_, ID_BACK_BUTTON, L"返回列表", 20, 20, 120, 40);
    controls_[ID_EDIT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_EDIT_BUTTON, L"编辑", 150, 20, 80, 40);
    controls_[ID_PRINT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_PRINT_BUTTON, L"打印标签", 240, 20, 120, 40);
    controls_[ID_SCRAP_BUTTON] = UIHelper::CreateButton(hWnd_, ID_SCRAP_BUTTON, L"报废设备", 370, 20, 120, 40);
    
    if (deviceId_ > 0) {
        LoadDevice();
    }
}

void DeviceDetailPage::OnPaint(HDC hdc) {
    RECT rcClient;
    GetClientRect(hWnd_, &rcClient);
    
    HFONT hTitleFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hLabelFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hValueFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hCodeFont = CreateFont(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");
    HFONT hParamLabelFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hParamValueFont = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    
    if (device_) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
        SetTextColor(hdc, RGB(0, 102, 204));
        RECT titleRect = {40, 80, rcClient.right - 40, 130};
        DrawText(hdc, device_->name.c_str(), -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetTextColor(hdc, RGB(0, 0, 0));
        SelectObject(hdc, hLabelFont);
        
        int y = 150;
        int labelX = 40;
        int valueX = 180;
        int lineHeight = 55;
        
        RECT labelRect, valueRect;
        
        SetRect(&labelRect, labelX, y, valueX - 15, y + lineHeight);
        DrawText(hdc, L"识别码:", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetRect(&valueRect, valueX, y, rcClient.right - 40, y + lineHeight);
        SelectObject(hdc, hCodeFont);
        SetTextColor(hdc, RGB(51, 51, 51));
        DrawText(hdc, device_->code.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SetTextColor(hdc, RGB(0, 0, 0));
        SelectObject(hdc, hLabelFont);
        
        y += lineHeight;
        SetRect(&labelRect, labelX, y, valueX - 15, y + lineHeight);
        DrawText(hdc, L"状态:", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetRect(&valueRect, valueX, y, rcClient.right - 40, y + lineHeight);
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
        SetRect(&labelRect, labelX, y, valueX - 15, y + lineHeight);
        DrawText(hdc, L"创建时间:", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetRect(&valueRect, valueX, y, rcClient.right - 40, y + lineHeight);
        SelectObject(hdc, hValueFont);
        DrawText(hdc, device_->created_at.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hLabelFont);
        
        y += lineHeight;
        SetRect(&labelRect, labelX, y, valueX - 15, y + lineHeight);
        DrawText(hdc, L"更新时间:", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        
        SetRect(&valueRect, valueX, y, rcClient.right - 40, y + lineHeight);
        SelectObject(hdc, hValueFont);
        DrawText(hdc, device_->updated_at.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hLabelFont);
        
        y += lineHeight + 20;
        
        // 绘制设备参数区域标题
        SetRect(&labelRect, labelX, y, rcClient.right - 40, y + lineHeight);
        SelectObject(hdc, hLabelFont);
        DrawText(hdc, L"设备参数:", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        y += lineHeight + 10;
        
        // 绘制参数表单
        DrawParamsForm(hdc, labelX, y, rcClient.right - 40);
        
        DrawBarcode(hdc);
        
        SelectObject(hdc, hOldFont);
    } else {
        HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
        RECT rect = {40, 100, rcClient.right - 40, 200};
        DrawText(hdc, L"设备不存在或已被删除", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hOldFont);
    }
    
    DeleteObject(hTitleFont);
    DeleteObject(hLabelFont);
    DeleteObject(hValueFont);
    DeleteObject(hCodeFont);
    DeleteObject(hParamLabelFont);
    DeleteObject(hParamValueFont);
}

void DeviceDetailPage::DrawParamsForm(HDC hdc, int x, int y, int right) {
    if (!device_ || device_->params.empty() || device_->params == L"{}") {
        HFONT hEmptyFont = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                     GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                     DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hEmptyFont);
        SetTextColor(hdc, RGB(128, 128, 128));
        RECT emptyRect = {x, y, right, y + 40};
        DrawText(hdc, L"暂无参数", -1, &emptyRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SetTextColor(hdc, RGB(0, 0, 0));
        SelectObject(hdc, hOldFont);
        DeleteObject(hEmptyFont);
        return;
    }
    
    HFONT hParamLabelFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hParamValueFont = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hParamLabelFont);
    
    try {
        std::string paramsUtf8 = WStringToString(device_->params);
        json j = json::parse(paramsUtf8);
        
        auto fields = DeviceRepository::Instance().GetAllParamFields();
        
        int paramY = y;
        int labelWidth = 160;
        int lineHeight = 45;
        int valueX = x + labelWidth + 15;
        
        for (const auto& field : fields) {
            std::string keyUtf8 = WStringToString(field.field_key);
            if (!j.contains(keyUtf8)) continue;
            
            std::string valueStr = j[keyUtf8].get<std::string>();
            std::wstring value = StringToWString(valueStr);
            if (value.empty()) value = L"-";
            
            // 绘制标签
            RECT labelRect = {x, paramY, x + labelWidth, paramY + lineHeight};
            SelectObject(hdc, hParamLabelFont);
            SetTextColor(hdc, RGB(80, 80, 80));
            DrawText(hdc, field.field_label.c_str(), -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            
            // 绘制值
            RECT valueRect = {valueX, paramY, right, paramY + lineHeight};
            SelectObject(hdc, hParamValueFont);
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawText(hdc, value.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            
            paramY += lineHeight;
        }
        
        // 显示未在字段定义中的额外参数
        for (auto& [key, value] : j.items()) {
            std::string keyWStr = key;
            bool found = false;
            for (const auto& field : fields) {
                if (WStringToString(field.field_key) == keyWStr) {
                    found = true;
                    break;
                }
            }
            if (found) continue;
            
            std::string valueStr = value.get<std::string>();
            std::wstring valueW = StringToWString(valueStr);
            if (valueW.empty()) valueW = L"-";
            
            std::wstring label = StringToWString(key) + L":";
            
            RECT labelRect = {x, paramY, x + labelWidth, paramY + lineHeight};
            SelectObject(hdc, hParamLabelFont);
            SetTextColor(hdc, RGB(80, 80, 80));
            DrawText(hdc, label.c_str(), -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            
            RECT valueRect = {valueX, paramY, right, paramY + lineHeight};
            SelectObject(hdc, hParamValueFont);
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawText(hdc, valueW.c_str(), -1, &valueRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            
            paramY += lineHeight;
        }
        
    } catch (...) {
        // 解析失败时回退到原始 JSON 显示
        RECT paramsRect = {x, y, right, y + 200};
        SelectObject(hdc, hParamValueFont);
        SetTextColor(hdc, RGB(0, 0, 0));
        DrawText(hdc, device_->params.c_str(), -1, &paramsRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EDITCONTROL);
    }
    
    SetTextColor(hdc, RGB(0, 0, 0));
    SelectObject(hdc, hOldFont);
    DeleteObject(hParamLabelFont);
    DeleteObject(hParamValueFont);
}

void DeviceDetailPage::DrawBarcode(HDC hdc) {
    if (!barcode_ || barcode_->width <= 0 || barcode_->height <= 0) return;

    RECT rcClient;
    GetClientRect(hWnd_, &rcClient);

    int displayWidth = 280;
    int displayHeight = 140;
    int x = rcClient.right - displayWidth - 40;
    int y = 120;

    RECT barcodeRect = {x - 10, y - 10, x + displayWidth + 10, y + displayHeight + 60};
    FillRect(hdc, &barcodeRect, (HBRUSH)GetStockObject(WHITE_BRUSH));

    HBITMAP hBitmap = barcode_->ToHBITMAP(hdc);
    if (hBitmap) {
        HDC hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

        int drawWidth = barcode_->width;
        int drawHeight = barcode_->height;
        int drawX = x + (displayWidth - drawWidth) / 2;
        int drawY = y + (displayHeight - drawHeight) / 2;

        if (drawWidth > displayWidth) {
            float ratio = (float)displayWidth / drawWidth;
            drawWidth = displayWidth;
            drawHeight = (int)(barcode_->height * ratio);
            drawX = x;
            drawY = y + (displayHeight - drawHeight) / 2;
        }

        SetStretchBltMode(hdc, STRETCH_HALFTONE);
        StretchBlt(hdc, drawX, drawY, drawWidth, drawHeight,
                   hMemDC, 0, 0, barcode_->width, barcode_->height, SRCCOPY);

        SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
    }

    HFONT hNameFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei");
    HFONT hCodeFont = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");

    RECT nameRect = {x, y + displayHeight + 5, x + displayWidth, y + displayHeight + 35};
    HFONT hOldFont = (HFONT)SelectObject(hdc, hNameFont);
    DrawText(hdc, device_->name.c_str(), -1, &nameRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    RECT codeRect = {x, y + displayHeight + 35, x + displayWidth, y + displayHeight + 65};
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
        barcode_ = BarcodeGenerator::Instance().GenerateCode128(device_->code, 280, 120);
        
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
