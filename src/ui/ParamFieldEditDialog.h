#pragma once

#include "ui/WindowBase.h"
#include "core/common.h"
#include <functional>

class ParamFieldEditDialog : public WindowBase {
public:
    ParamFieldEditDialog();
    
    void SetField(const ParamField& field);
    ParamField GetField() const;
    void SetOnSave(std::function<void(const ParamField&)> callback);
    
    enum ControlId {
        ID_KEY_EDIT = 3001,
        ID_LABEL_EDIT,
        ID_TYPE_COMBO,
        ID_REQUIRED_CHECK,
        ID_SAVE_BUTTON,
        ID_CANCEL_BUTTON,
        ID_KEY_LABEL,
        ID_LABEL_LABEL,
        ID_TYPE_LABEL,
        ID_REQUIRED_LABEL
    };
    
protected:
    void OnCreate() override;
    void OnSize(int cx, int cy) override;
    void OnCommand(WPARAM wParam, LPARAM lParam) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    
private:
    void OnSave();
    void OnCancel();
    
    ParamField field_;
    std::function<void(const ParamField&)> onSave_;
    bool isEdit_ = false;
};
