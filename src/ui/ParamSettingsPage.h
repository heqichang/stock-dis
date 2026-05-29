#pragma once

#include "ui/WindowBase.h"
#include "core/common.h"

class ParamSettingsPage : public WindowBase {
public:
    ParamSettingsPage();
    
    enum ControlId {
        ID_LIST_VIEW = 5001,
        ID_ADD_BUTTON,
        ID_EDIT_BUTTON,
        ID_DELETE_BUTTON,
        ID_UP_BUTTON,
        ID_DOWN_BUTTON,
        ID_CLOSE_BUTTON
    };
    
    void RefreshData();
    
protected:
    void OnCreate() override;
    void OnSize(int cx, int cy) override;
    void OnCommand(WPARAM wParam, LPARAM lParam) override;
    void OnMeasureItem(WPARAM idCtrl, LPMEASUREITEMSTRUCT lpMeasureItem) override;
    void OnDrawItem(LPDRAWITEMSTRUCT lpDrawItem) override;
    
private:
    void LoadData();
    void OnAdd();
    void OnEdit();
    void OnDelete();
    void OnMoveUp();
    void OnMoveDown();
    void OnClose();
    
    std::vector<ParamField> fields_;
};
