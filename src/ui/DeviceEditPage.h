#pragma once

#include "ui/WindowBase.h"
#include "core/common.h"

class DeviceEditPage : public WindowBase {
public:
    DeviceEditPage();
    
    void SetEditDevice(int64_t deviceId);
    void Clear();
    
    enum ControlId {
        ID_NAME_EDIT = 2001,
        ID_CODE_EDIT,
        ID_PARAMS_EDIT,
        ID_SAVE_BUTTON,
        ID_CANCEL_BUTTON,
        ID_NAME_LABEL,
        ID_CODE_LABEL,
        ID_PARAMS_LABEL,
        ID_TIPS_LABEL
    };
    
protected:
    void OnCreate() override;
    void OnSize(int cx, int cy) override;
    void OnCommand(WPARAM wParam, LPARAM lParam) override;
    
private:
    void OnSave();
    void OnCancel();
    void ValidateParams();
    
    bool isEdit_ = false;
    int64_t editDeviceId_ = 0;
    std::wstring originalCode_;
};
