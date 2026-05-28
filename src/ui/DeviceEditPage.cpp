#include "DeviceEditPage.h"
#include "ui/UIHelper.h"
#include "db/DeviceRepository.h"
#include "barcode/BarcodeGenerator.h"
#include "print/PrintService.h"
#include <sstream>

DeviceEditPage::DeviceEditPage() {}

void DeviceEditPage::OnCreate() {
    controls_[ID_NAME_LABEL] = UIHelper::CreateStatic(hWnd_, ID_NAME_LABEL, L"设备名称:", 30, 30, 100, 25);
    controls_[ID_NAME_EDIT] = UIHelper::CreateEdit(hWnd_, ID_NAME_EDIT, L"", 140, 30, 300, 30);
    
    controls_[ID_CODE_LABEL] = UIHelper::CreateStatic(hWnd_, ID_CODE_LABEL, L"识别码:", 30, 80, 100, 25);
    controls_[ID_CODE_EDIT] = UIHelper::CreateEdit(hWnd_, ID_CODE_EDIT, L"", 140, 80, 300, 30, ES_READONLY);
    
    controls_[ID_PARAMS_LABEL] = UIHelper::CreateStatic(hWnd_, ID_PARAMS_LABEL, L"设备参数 (JSON格式):", 30, 130, 200, 25);
    controls_[ID_PARAMS_EDIT] = UIHelper::CreateEdit(hWnd_, ID_PARAMS_EDIT, L"{}", 30, 160, 540, 200, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL);
    
    controls_[ID_TIPS_LABEL] = UIHelper::CreateStatic(hWnd_, ID_TIPS_LABEL, 
        L"提示: 请输入有效的JSON格式，例如：{\"品牌\":\"华为\",\"型号\":\"MateBook X Pro\"}", 
        30, 370, 540, 25, SS_CENTER);
    
    controls_[ID_SAVE_BUTTON] = UIHelper::CreateButton(hWnd_, ID_SAVE_BUTTON, L"保存", 200, 410, 100, 35);
    controls_[ID_CANCEL_BUTTON] = UIHelper::CreateButton(hWnd_, ID_CANCEL_BUTTON, L"取消", 350, 410, 100, 35);
    
    if (!isEdit_) {
        std::wstring code = GenerateUniqueCode();
        while (DeviceRepository::Instance().ExistsByCode(code)) {
            code = GenerateUniqueCode();
        }
        UIHelper::SetEditText(controls_[ID_CODE_EDIT], code);
    }
}

void DeviceEditPage::OnSize(int cx, int cy) {
    if (cx < 600 || cy < 500) return;
    
    int editWidth = cx - 200;
    if (editWidth < 300) editWidth = 300;
    
    SetWindowPos(controls_[ID_NAME_EDIT], nullptr, 140, 30, editWidth, 30, SWP_NOZORDER);
    SetWindowPos(controls_[ID_CODE_EDIT], nullptr, 140, 80, editWidth, 30, SWP_NOZORDER);
    
    int paramsWidth = cx - 60;
    if (paramsWidth < 300) paramsWidth = 300;
    int paramsHeight = cy - 300;
    if (paramsHeight < 150) paramsHeight = 150;
    
    SetWindowPos(controls_[ID_PARAMS_EDIT], nullptr, 30, 160, paramsWidth, paramsHeight, SWP_NOZORDER);
    SetWindowPos(controls_[ID_TIPS_LABEL], nullptr, 30, 160 + paramsHeight + 10, paramsWidth, 25, SWP_NOZORDER);
    
    int btnY = 160 + paramsHeight + 45;
    int btnX1 = (cx - 220) / 2;
    int btnX2 = btnX1 + 120;
    
    SetWindowPos(controls_[ID_SAVE_BUTTON], nullptr, btnX1, btnY, 100, 35, SWP_NOZORDER);
    SetWindowPos(controls_[ID_CANCEL_BUTTON], nullptr, btnX2, btnY, 100, 35, SWP_NOZORDER);
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
            UIHelper::SetEditText(controls_[ID_PARAMS_EDIT], device->params);
            EnableWindow(controls_[ID_CODE_EDIT], FALSE);
        }
        originalCode_ = device->code;
    }
}

void DeviceEditPage::Clear() {
    isEdit_ = false;
    editDeviceId_ = 0;
    originalCode_.clear();
    
    if (controls_[ID_NAME_EDIT]) {
        UIHelper::SetEditText(controls_[ID_NAME_EDIT], L"");
        UIHelper::SetEditText(controls_[ID_PARAMS_EDIT], L"{}");
        EnableWindow(controls_[ID_CODE_EDIT], TRUE);
        
        std::wstring code = GenerateUniqueCode();
        while (DeviceRepository::Instance().ExistsByCode(code)) {
            code = GenerateUniqueCode();
        }
        UIHelper::SetEditText(controls_[ID_CODE_EDIT], code);
    }
}

void DeviceEditPage::OnSave() {
    std::wstring name = UIHelper::GetEditText(controls_[ID_NAME_EDIT]);
    std::wstring code = UIHelper::GetEditText(controls_[ID_CODE_EDIT]);
    std::wstring params = UIHelper::GetEditText(controls_[ID_PARAMS_EDIT]);
    
    if (name.empty()) {
        UIHelper::ShowMessageBox(hWnd_, L"请输入设备名称", L"提示");
        SetFocus(controls_[ID_NAME_EDIT]);
        return;
    }
    
    if (code.empty()) {
        UIHelper::ShowMessageBox(hWnd_, L"识别码不能为空", L"提示");
        return;
    }
    
    if (params.empty()) {
        params = L"{}";
    }
    
    if (!isEdit_ && DeviceRepository::Instance().ExistsByCode(code)) {
        UIHelper::ShowMessageBox(hWnd_, L"该识别码已存在，请重新生成或手动输入", L"提示");
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
                label.barcode = BarcodeGenerator::Instance().GenerateCode128(device.code);
                
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
