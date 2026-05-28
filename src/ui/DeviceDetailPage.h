#pragma once

#include "ui/WindowBase.h"
#include "core/common.h"
#include "barcode/BarcodeGenerator.h"

class DeviceDetailPage : public WindowBase {
public:
    DeviceDetailPage();
    
    void SetDevice(int64_t deviceId);
    
    enum ControlId {
        ID_BACK_BUTTON = 3001,
        ID_EDIT_BUTTON,
        ID_PRINT_BUTTON,
        ID_SCRAP_BUTTON,
        ID_BARCODE_STATIC
    };
    
protected:
    void OnCreate() override;
    void OnPaint() override;
    void OnSize(int cx, int cy) override;
    void OnCommand(WPARAM wParam, LPARAM lParam) override;
    
private:
    void LoadDevice();
    void OnBack();
    void OnEdit();
    void OnPrint();
    void OnScrap();
    void DrawBarcode(HDC hdc);
    
    int64_t deviceId_ = 0;
    std::optional<Device> device_;
    std::optional<BarcodeImage> barcode_;
};
