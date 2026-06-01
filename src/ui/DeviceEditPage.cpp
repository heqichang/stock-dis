#include "DeviceEditPage.h"
#include "ui/UIHelper.h"
#include "db/DeviceRepository.h"
#include "barcode/BarcodeGenerator.h"
#include "print/PrintService.h"
#include "json.hpp"
#include <sstream>

using json = nlohmann::json;

DeviceEditPage::DeviceEditPage() {}

void DeviceEditPage::OnCreate() {
    controls_[ID_NAME_LABEL] = UIHelper::CreateStatic(hWnd_, ID_NAME_LABEL, L"设备名称:", 30, 35, 100, 28);
    controls_[ID_NAME_EDIT] = UIHelper::CreateEdit(hWnd_, ID_NAME_EDIT, L"", 140, 30, 300, 36);
    
    controls_[ID_CODE_LABEL] = UIHelper::CreateStatic(hWnd_, ID_CODE_LABEL, L"识别码:", 30, 90, 100, 28);
    controls_[ID_CODE_EDIT] = UIHelper::CreateEdit(hWnd_, ID_CODE_EDIT, L"", 140, 85, 300, 36, ES_READONLY);
    
    CreateParamForm();
    
    controls_[ID_TIPS_LABEL] = UIHelper::CreateStatic(hWnd_, ID_TIPS_LABEL, 
        L"提示: 请填写设备信息，带*为必填项", 
        30, 410, 540, 28, SS_CENTER);
    
    controls_[ID_SAVE_BUTTON] = UIHelper::CreateButton(hWnd_, ID_SAVE_BUTTON, L"保存", 200, 455, 100, 40);
    controls_[ID_CANCEL_BUTTON] = UIHelper::CreateButton(hWnd_, ID_CANCEL_BUTTON, L"取消", 350, 455, 100, 40);
    
    if (!isEdit_) {
        std::wstring code = GenerateUniqueCode();
        UIHelper::SetEditText(controls_[ID_CODE_EDIT], code);
    }
}

void DeviceEditPage::CreateParamForm() {
    for (auto& ctrl : paramControls_) {
        if (ctrl.hLabel) DestroyWindow(ctrl.hLabel);
        if (ctrl.hEdit) DestroyWindow(ctrl.hEdit);
    }
    paramControls_.clear();
    
    auto fields = DeviceRepository::Instance().GetAllParamFields();
    int yOffset = 145;
    int controlId = ID_PARAM_FIRST;
    
    for (size_t i = 0; i < fields.size(); ++i) {
        const auto& field = fields[i];
        
        ParamFormControl ctrl;
        ctrl.field = field;
        ctrl.labelId = controlId++;
        ctrl.editId = controlId++;
        
        std::wstring labelText = field.field_label;
        if (field.is_required) {
            labelText += L"*";
        }
        labelText += L":";
        
        ctrl.hLabel = UIHelper::CreateStatic(hWnd_, ctrl.labelId, labelText, 30, yOffset + 5, 100, 28);
        controls_[ctrl.labelId] = ctrl.hLabel;
        
        DWORD style = 0;
        if (field.field_type == ParamFieldType::NUMBER) {
            style = ES_NUMBER;
        }
        
        ctrl.hEdit = UIHelper::CreateEdit(hWnd_, ctrl.editId, L"", 140, yOffset, 300, 36, style);
        controls_[ctrl.editId] = ctrl.hEdit;
        
        paramControls_.push_back(ctrl);
        yOffset += 50;
    }
}

void DeviceEditPage::OnSize(int cx, int cy) {
    if (cx < 600 || cy < 500) return;
    
    int editWidth = cx - 200;
    if (editWidth < 300) editWidth = 300;
    
    SetWindowPos(controls_[ID_NAME_EDIT], nullptr, 140, 30, editWidth, 36, SWP_NOZORDER);
    SetWindowPos(controls_[ID_CODE_EDIT], nullptr, 140, 85, editWidth, 36, SWP_NOZORDER);
    
    int yOffset = 145;
    for (auto& ctrl : paramControls_) {
        SetWindowPos(ctrl.hEdit, nullptr, 140, yOffset, editWidth, 36, SWP_NOZORDER);
        yOffset += 50;
    }
    
    int tipsY = yOffset + 10;
    int paramsWidth = cx - 60;
    if (paramsWidth < 300) paramsWidth = 300;
    
    SetWindowPos(controls_[ID_TIPS_LABEL], nullptr, 30, tipsY, paramsWidth, 28, SWP_NOZORDER);
    
    int btnY = tipsY + 45;
    int btnX1 = (cx - 220) / 2;
    int btnX2 = btnX1 + 120;
    
    SetWindowPos(controls_[ID_SAVE_BUTTON], nullptr, btnX1, btnY, 100, 40, SWP_NOZORDER);
    SetWindowPos(controls_[ID_CANCEL_BUTTON], nullptr, btnX2, btnY, 100, 40, SWP_NOZORDER);
}

void DeviceEditPage::OnCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
        case ID_SAVE_BUTTON:
            OnSave();
            break;
        case ID_CANCEL_BUTTON:
            OnCancel();
            break;
    }
}

void DeviceEditPage::SetEditDevice(int64_t deviceId) {
    isEdit_ = true;
    editDeviceId_ = deviceId;
    
    auto device = DeviceRepository::Instance().GetById(deviceId);
    if (device) {
        if (controls_[ID_NAME_EDIT]) {
            UIHelper::SetEditText(controls_[ID_NAME_EDIT], device->name);
            UIHelper::SetEditText(controls_[ID_CODE_EDIT], device->code);
            EnableWindow(controls_[ID_CODE_EDIT], FALSE);
        }
        originalCode_ = device->code;
        originalParams_ = device->params;
        
        LoadParamValues();
    }
}

void DeviceEditPage::LoadParamValues() {
    if (originalParams_.empty()) return;
    
    try {
        json j = json::parse(WStringToString(originalParams_));
        if (!j.is_object()) return;
        
        for (auto& ctrl : paramControls_) {
            std::string key = WStringToString(ctrl.field.field_key);
            if (j.contains(key)) {
                std::wstring value;
                if (j[key].is_string()) {
                    value = StringToWString(j[key].get<std::string>());
                } else {
                    value = StringToWString(j[key].dump());
                }
                UIHelper::SetEditText(ctrl.hEdit, value);
            }
        }
    } catch (...) {
    }
}

std::wstring DeviceEditPage::CollectParams() {
    json j;
    
    if (!originalParams_.empty()) {
        try {
            j = json::parse(WStringToString(originalParams_));
        } catch (...) {
            j = json::object();
        }
    }
    
    for (auto& ctrl : paramControls_) {
        std::wstring value = UIHelper::GetEditText(ctrl.hEdit);
        std::string key = WStringToString(ctrl.field.field_key);
        
        if (!value.empty()) {
            if (ctrl.field.field_type == ParamFieldType::NUMBER) {
                try {
                    double num = std::stod(WStringToString(value));
                    j[key] = num;
                } catch (...) {
                    j[key] = WStringToString(value);
                }
            } else {
                j[key] = WStringToString(value);
            }
        } else if (j.contains(key)) {
            j.erase(key);
        }
    }
    
    return StringToWString(j.dump());
}

void DeviceEditPage::Clear() {
    isEdit_ = false;
    editDeviceId_ = 0;
    originalCode_.clear();
    originalParams_.clear();
    
    if (controls_[ID_NAME_EDIT]) {
        UIHelper::SetEditText(controls_[ID_NAME_EDIT], L"");
        EnableWindow(controls_[ID_CODE_EDIT], FALSE);
        
        std::wstring code = GenerateUniqueCode();
        UIHelper::SetEditText(controls_[ID_CODE_EDIT], code);
        
        CreateParamForm();
    }
}

void DeviceEditPage::OnSave() {
    std::wstring name = UIHelper::GetEditText(controls_[ID_NAME_EDIT]);
    std::wstring code = UIHelper::GetEditText(controls_[ID_CODE_EDIT]);
    std::wstring params = CollectParams();
    
    if (name.empty()) {
        UIHelper::ShowMessageBox(hWnd_, L"请输入设备名称", L"提示");
        SetFocus(controls_[ID_NAME_EDIT]);
        return;
    }
    
    if (code.empty()) {
        UIHelper::ShowMessageBox(hWnd_, L"识别码不能为空", L"提示");
        return;
    }
    
    for (auto& ctrl : paramControls_) {
        if (ctrl.field.is_required) {
            std::wstring value = UIHelper::GetEditText(ctrl.hEdit);
            if (value.empty()) {
                std::wstring msg = L"请填写必填项: " + ctrl.field.field_label;
                UIHelper::ShowMessageBox(hWnd_, msg, L"提示");
                SetFocus(ctrl.hEdit);
                return;
            }
        }
    }
    
    if (!isEdit_ && DeviceRepository::Instance().ExistsByCode(code)) {
        UIHelper::ShowMessageBox(hWnd_, L"该识别码已存在，请重新生成", L"提示");
        return;
    }
    
    if (isEdit_) {
        Device device;
        device.id = editDeviceId_;
        device.name = name;
        device.code = originalCode_;
        device.params = params;
        
        if (DeviceRepository::Instance().Update(device)) {
            UIHelper::ShowMessageBox(hWnd_, L"更新成功", L"提示");
            SendMessage(hParent_, WM_COMMAND, 10, (LPARAM)editDeviceId_);
        } else {
            UIHelper::ShowMessageBox(hWnd_, L"更新失败", L"错误", MB_OK | MB_ICONERROR);
        }
    } else {
        Device device;
        device.name = name;
        device.code = code;
        device.status = DeviceStatus::IN_USE;
        device.params = params;
        
        if (DeviceRepository::Instance().Add(device)) {
            if (IDYES == UIHelper::ShowMessageBox(hWnd_, L"添加成功，是否立即打印条形码？", L"提示", MB_YESNO | MB_ICONQUESTION)) {
                PrintLabel label;
                label.name = device.name;
                label.code = device.code;
                label.barcode = BarcodeGenerator::Instance().GenerateCode128(device.code, 280, 120);
                
                std::vector<PrintLabel> labels;
                labels.push_back(label);
                PrintService::Instance().PrintLabels(labels);
            }
            
            SendMessage(hParent_, WM_COMMAND, 10, (LPARAM)device.id);
        } else {
            UIHelper::ShowMessageBox(hWnd_, L"添加失败", L"错误", MB_OK | MB_ICONERROR);
        }
    }
}

void DeviceEditPage::OnCancel() {
    Clear();
    SendMessage(hParent_, WM_COMMAND, 11, 0);
}
