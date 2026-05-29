#include "ParamFieldEditDialog.h"
#include "ui/UIHelper.h"
#include "db/DeviceRepository.h"

ParamFieldEditDialog::ParamFieldEditDialog() {}

void ParamFieldEditDialog::SetField(const ParamField& field) {
    field_ = field;
    isEdit_ = field.id > 0;
    
    if (controls_[ID_KEY_EDIT]) {
        UIHelper::SetEditText(controls_[ID_KEY_EDIT], field.field_key);
        UIHelper::SetEditText(controls_[ID_LABEL_EDIT], field.field_label);
        
        int typeIndex = 0;
        switch (field.field_type) {
            case ParamFieldType::TEXT: typeIndex = 0; break;
            case ParamFieldType::NUMBER: typeIndex = 1; break;
            case ParamFieldType::DATE: typeIndex = 2; break;
            case ParamFieldType::SELECT: typeIndex = 3; break;
        }
        UIHelper::SetComboBoxSelectedIndex(controls_[ID_TYPE_COMBO], typeIndex);
        
        SendMessage(controls_[ID_REQUIRED_CHECK], BM_SETCHECK, field.is_required ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

ParamField ParamFieldEditDialog::GetField() const {
    return field_;
}

void ParamFieldEditDialog::SetOnSave(std::function<void(const ParamField&)> callback) {
    onSave_ = callback;
}

void ParamFieldEditDialog::OnCreate() {
    controls_[ID_KEY_LABEL] = UIHelper::CreateStatic(hWnd_, ID_KEY_LABEL, L"字段标识:", 30, 30, 100, 28);
    controls_[ID_KEY_EDIT] = UIHelper::CreateEdit(hWnd_, ID_KEY_EDIT, L"", 140, 25, 260, 36);
    
    controls_[ID_LABEL_LABEL] = UIHelper::CreateStatic(hWnd_, ID_LABEL_LABEL, L"显示名称:", 30, 85, 100, 28);
    controls_[ID_LABEL_EDIT] = UIHelper::CreateEdit(hWnd_, ID_LABEL_EDIT, L"", 140, 80, 260, 36);
    
    controls_[ID_TYPE_LABEL] = UIHelper::CreateStatic(hWnd_, ID_TYPE_LABEL, L"字段类型:", 30, 140, 100, 28);
    controls_[ID_TYPE_COMBO] = UIHelper::CreateComboBox(hWnd_, ID_TYPE_COMBO, 140, 135, 260, 200);
    UIHelper::AddComboBoxItem(controls_[ID_TYPE_COMBO], L"文本", (int)ParamFieldType::TEXT);
    UIHelper::AddComboBoxItem(controls_[ID_TYPE_COMBO], L"数字", (int)ParamFieldType::NUMBER);
    UIHelper::AddComboBoxItem(controls_[ID_TYPE_COMBO], L"日期", (int)ParamFieldType::DATE);
    UIHelper::AddComboBoxItem(controls_[ID_TYPE_COMBO], L"选择", (int)ParamFieldType::SELECT);
    UIHelper::SetComboBoxSelectedIndex(controls_[ID_TYPE_COMBO], 0);
    
    controls_[ID_REQUIRED_LABEL] = UIHelper::CreateStatic(hWnd_, ID_REQUIRED_LABEL, L"必填:", 30, 195, 100, 28);
    controls_[ID_REQUIRED_CHECK] = UIHelper::CreateCheckBox(hWnd_, ID_REQUIRED_CHECK, L"", 140, 190, 36, 36);
    
    controls_[ID_SAVE_BUTTON] = UIHelper::CreateButton(hWnd_, ID_SAVE_BUTTON, L"保存", 100, 255, 100, 40);
    controls_[ID_CANCEL_BUTTON] = UIHelper::CreateButton(hWnd_, ID_CANCEL_BUTTON, L"取消", 250, 255, 100, 40);
    
    if (isEdit_) {
        SetField(field_);
    }
}

void ParamFieldEditDialog::OnSize(int cx, int cy) {}

void ParamFieldEditDialog::OnCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
        case ID_SAVE_BUTTON:
            OnSave();
            break;
        case ID_CANCEL_BUTTON:
            OnCancel();
            break;
    }
}

LRESULT ParamFieldEditDialog::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_CLOSE) {
        DestroyWindow(hWnd_);
        return 0;
    }
    return WindowBase::HandleMessage(uMsg, wParam, lParam);
}

void ParamFieldEditDialog::OnSave() {
    std::wstring key = UIHelper::GetEditText(controls_[ID_KEY_EDIT]);
    std::wstring label = UIHelper::GetEditText(controls_[ID_LABEL_EDIT]);
    int typeData = UIHelper::GetComboBoxSelectedData(controls_[ID_TYPE_COMBO]);
    bool isRequired = SendMessage(controls_[ID_REQUIRED_CHECK], BM_GETCHECK, 0, 0) == BST_CHECKED;
    
    if (key.empty()) {
        UIHelper::ShowMessageBox(hWnd_, L"请输入字段标识", L"提示");
        SetFocus(controls_[ID_KEY_EDIT]);
        return;
    }
    
    if (label.empty()) {
        UIHelper::ShowMessageBox(hWnd_, L"请输入显示名称", L"提示");
        SetFocus(controls_[ID_LABEL_EDIT]);
        return;
    }
    
    ParamField existing = DeviceRepository::Instance().GetParamFieldByKey(key);
    if (existing.id > 0 && existing.id != field_.id) {
        UIHelper::ShowMessageBox(hWnd_, L"字段标识已存在", L"提示");
        SetFocus(controls_[ID_KEY_EDIT]);
        return;
    }
    
    field_.field_key = key;
    field_.field_label = label;
    field_.field_type = (ParamFieldType)typeData;
    field_.is_required = isRequired;
    
    if (onSave_) {
        onSave_(field_);
    }
    DestroyWindow(hWnd_);
}

void ParamFieldEditDialog::OnCancel() {
    DestroyWindow(hWnd_);
}
