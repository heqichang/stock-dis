#pragma once

#include "ui/WindowBase.h"
#include "core/common.h"
#include <vector>

struct ParamFormControl {
    int labelId;
    int editId;
    ParamField field;
    HWND hLabel;
    HWND hEdit;
};

class DeviceEditPage : public WindowBase {
public:
    DeviceEditPage();
    
    void SetEditDevice(int64_t deviceId);
    void Clear();
    
    enum ControlId {
        ID_NAME_EDIT = 2001,
        ID_CODE_EDIT,
        ID_SAVE_BUTTON,
        ID_CANCEL_BUTTON,
        ID_NAME_LABEL,
        ID_CODE_LABEL,
        ID_TIPS_LABEL,
        ID_PARAM_FIRST = 2100
    };
    
protected:
    void OnCreate() override;
    void OnSize(int cx, int cy) override;
    void OnCommand(WPARAM wParam, LPARAM lParam) override;
    
private:
    void OnSave();
    void OnCancel();
    void CreateParamForm();
    void LoadParamValues();
    std::wstring CollectParams();
    
    bool isEdit_ = false;
    int64_t editDeviceId_ = 0;
    std::wstring originalCode_;
    std::wstring originalParams_;
    std::vector<ParamFormControl> paramControls_;
};
