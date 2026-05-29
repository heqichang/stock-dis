#include "ParamSettingsPage.h"
#include "ui/UIHelper.h"
#include "ui/ParamFieldEditDialog.h"
#include "db/DeviceRepository.h"
#include <sstream>

ParamSettingsPage::ParamSettingsPage() {}

void ParamSettingsPage::OnCreate() {
    UIHelper::CreateStatic(hWnd_, -1, L"参数设置 - 管理设备参数字段", 20, 15, 300, 28);
    
    controls_[ID_LIST_VIEW] = UIHelper::CreateListView(hWnd_, ID_LIST_VIEW, 20, 55, 800, 450);
    
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 0, L"ID", 60);
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 1, L"字段标识", 150);
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 2, L"显示名称", 150);
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 3, L"类型", 100);
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 4, L"排序", 60);
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 5, L"必填", 60);
    
    controls_[ID_ADD_BUTTON] = UIHelper::CreateButton(hWnd_, ID_ADD_BUTTON, L"新增", 840, 55, 100, 36);
    controls_[ID_EDIT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_EDIT_BUTTON, L"编辑", 840, 105, 100, 36);
    controls_[ID_DELETE_BUTTON] = UIHelper::CreateButton(hWnd_, ID_DELETE_BUTTON, L"删除", 840, 155, 100, 36);
    controls_[ID_UP_BUTTON] = UIHelper::CreateButton(hWnd_, ID_UP_BUTTON, L"上移", 840, 205, 100, 36);
    controls_[ID_DOWN_BUTTON] = UIHelper::CreateButton(hWnd_, ID_DOWN_BUTTON, L"下移", 840, 255, 100, 36);
    controls_[ID_CLOSE_BUTTON] = UIHelper::CreateButton(hWnd_, ID_CLOSE_BUTTON, L"关闭", 840, 465, 100, 36);
    
    LoadData();
}

void ParamSettingsPage::OnSize(int cx, int cy) {
    if (cx < 200 || cy < 200) return;
    
    int listWidth = cx - 160;
    int listHeight = cy - 80;
    SetWindowPos(controls_[ID_LIST_VIEW], nullptr, 20, 55, listWidth, listHeight, SWP_NOZORDER);
    
    int btnX = cx - 130;
    SetWindowPos(controls_[ID_ADD_BUTTON], nullptr, btnX, 55, 100, 36, SWP_NOZORDER);
    SetWindowPos(controls_[ID_EDIT_BUTTON], nullptr, btnX, 105, 100, 36, SWP_NOZORDER);
    SetWindowPos(controls_[ID_DELETE_BUTTON], nullptr, btnX, 155, 100, 36, SWP_NOZORDER);
    SetWindowPos(controls_[ID_UP_BUTTON], nullptr, btnX, 205, 100, 36, SWP_NOZORDER);
    SetWindowPos(controls_[ID_DOWN_BUTTON], nullptr, btnX, 255, 100, 36, SWP_NOZORDER);
    SetWindowPos(controls_[ID_CLOSE_BUTTON], nullptr, btnX, cy - 80, 100, 36, SWP_NOZORDER);
}

void ParamSettingsPage::OnCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
        case ID_ADD_BUTTON:
            OnAdd();
            break;
        case ID_EDIT_BUTTON:
            OnEdit();
            break;
        case ID_DELETE_BUTTON:
            OnDelete();
            break;
        case ID_UP_BUTTON:
            OnMoveUp();
            break;
        case ID_DOWN_BUTTON:
            OnMoveDown();
            break;
        case ID_CLOSE_BUTTON:
            OnClose();
            break;
    }
}

void ParamSettingsPage::OnMeasureItem(WPARAM idCtrl, LPMEASUREITEMSTRUCT lpMeasureItem) {
    if (idCtrl == ID_LIST_VIEW) {
        lpMeasureItem->itemHeight = 28;
    }
}

void ParamSettingsPage::OnDrawItem(LPDRAWITEMSTRUCT lpDrawItem) {
    if (lpDrawItem->CtlType != ODT_LISTVIEW) return;
    
    HWND hListView = GetDlgItem(hWnd_, ID_LIST_VIEW);
    if (!hListView) return;
    
    HDC hdc = lpDrawItem->hDC;
    RECT rcItem = lpDrawItem->rcItem;
    int nItem = lpDrawItem->itemID;
    
    if (nItem < 0) return;
    
    COLORREF textColor = GetSysColor(COLOR_WINDOWTEXT);
    COLORREF bkColor = GetSysColor(COLOR_WINDOW);
    HBRUSH hBrush = nullptr;
    
    if (lpDrawItem->itemState & ODS_SELECTED) {
        textColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
        bkColor = GetSysColor(COLOR_HIGHLIGHT);
        hBrush = CreateSolidBrush(bkColor);
    } else {
        hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    }
    
    SetTextColor(hdc, textColor);
    SetBkColor(hdc, bkColor);
    FillRect(hdc, &rcItem, hBrush);
    
    if (lpDrawItem->itemState & ODS_SELECTED) {
        DeleteObject(hBrush);
    }
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, UIHelper::GetFont(UIHelper::FontType::List));
    
    TCHAR buf[256];
    
    for (int iCol = 0; iCol < 6; ++iCol) {
        LVITEM lvi = {};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = nItem;
        lvi.iSubItem = iCol;
        lvi.pszText = buf;
        lvi.cchTextMax = _countof(buf);
        ListView_GetItem(hListView, &lvi);
        
        RECT rcCol;
        ListView_GetSubItemRect(hListView, nItem, iCol, LVIR_BOUNDS, &rcCol);
        rcCol.left += 5;
        rcCol.right -= 5;
        
        DrawText(hdc, buf, -1, &rcCol, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
    
    SelectObject(hdc, hOldFont);
}

void ParamSettingsPage::LoadData() {
    fields_ = DeviceRepository::Instance().GetAllParamFields();
    
    UIHelper::ClearListView(controls_[ID_LIST_VIEW]);
    
    for (size_t i = 0; i < fields_.size(); ++i) {
        const auto& field = fields_[i];
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 0, std::to_wstring(field.id), (LPARAM)field.id);
        
        std::wstring typeStr;
        switch (field.field_type) {
            case ParamFieldType::TEXT: typeStr = L"文本"; break;
            case ParamFieldType::NUMBER: typeStr = L"数字"; break;
            case ParamFieldType::DATE: typeStr = L"日期"; break;
            case ParamFieldType::SELECT: typeStr = L"下拉"; break;
        }
        
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 1, field.field_key);
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 2, field.field_label);
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 3, typeStr);
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 4, std::to_wstring(field.sort_order));
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 5, field.is_required ? L"是" : L"否");
    }
}

void ParamSettingsPage::RefreshData() {
    LoadData();
}

void ParamSettingsPage::OnAdd() {
    ParamFieldEditDialog* dialog = new ParamFieldEditDialog();
    ParamField newField;
    newField.field_key = L"";
    newField.field_label = L"";
    newField.field_type = ParamFieldType::TEXT;
    newField.sort_order = (int)fields_.size() + 1;
    newField.is_required = false;
    
    dialog->SetField(newField);
    dialog->SetOnSave([this](const ParamField& field) {
        ParamField newField = field;
        if (DeviceRepository::Instance().AddParamField(newField)) {
            LoadData();
            UIHelper::ShowMessageBox(hWnd_, L"新增成功", L"提示");
        } else {
            UIHelper::ShowMessageBox(hWnd_, L"新增失败", L"错误", MB_OK | MB_ICONERROR);
        }
    });
    
    RECT rcClient;
    GetClientRect(hWnd_, &rcClient);
    int dlgWidth = 450;
    int dlgHeight = 350;
    int x = (rcClient.right - dlgWidth) / 2;
    int y = (rcClient.bottom - dlgHeight) / 2;
    
    dialog->Create(hWnd_, L"新增字段", x, y, dlgWidth, dlgHeight);
}

void ParamSettingsPage::OnEdit() {
    int row = UIHelper::GetListViewSelectedRow(controls_[ID_LIST_VIEW]);
    if (row == -1) {
        UIHelper::ShowMessageBox(hWnd_, L"请先选择一个字段", L"提示");
        return;
    }
    
    int64_t id = (int64_t)UIHelper::GetListViewItemData(controls_[ID_LIST_VIEW], row);
    
    auto field = DeviceRepository::Instance().GetParamFieldById(id);
    if (!field) {
        UIHelper::ShowMessageBox(hWnd_, L"字段不存在", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    
    ParamFieldEditDialog* dialog = new ParamFieldEditDialog();
    dialog->SetField(*field);
    dialog->SetOnSave([this, dialog](const ParamField& f) {
        if (DeviceRepository::Instance().UpdateParamField(f)) {
            LoadData();
            UIHelper::ShowMessageBox(hWnd_, L"更新成功", L"提示");
        } else {
            UIHelper::ShowMessageBox(hWnd_, L"更新失败", L"错误", MB_OK | MB_ICONERROR);
        }
    });
    
    RECT rcClient;
    GetClientRect(hWnd_, &rcClient);
    int dlgWidth = 450;
    int dlgHeight = 350;
    int x = (rcClient.right - dlgWidth) / 2;
    int y = (rcClient.bottom - dlgHeight) / 2;
    
    dialog->Create(hWnd_, L"编辑字段", x, y, dlgWidth, dlgHeight);
}

void ParamSettingsPage::OnDelete() {
    int row = UIHelper::GetListViewSelectedRow(controls_[ID_LIST_VIEW]);
    if (row == -1) {
        UIHelper::ShowMessageBox(hWnd_, L"请先选择一个字段", L"提示");
        return;
    }
    
    if (!UIHelper::ShowConfirmDialog(hWnd_, L"确定要删除这个字段吗？", L"确认删除")) {
        return;
    }
    
    int64_t id = (int64_t)UIHelper::GetListViewItemData(controls_[ID_LIST_VIEW], row);
    
    if (DeviceRepository::Instance().DeleteParamField(id)) {
        LoadData();
        UIHelper::ShowMessageBox(hWnd_, L"删除成功", L"提示");
    } else {
        UIHelper::ShowMessageBox(hWnd_, L"删除失败", L"错误", MB_OK | MB_ICONERROR);
    }
}

void ParamSettingsPage::OnMoveUp() {
    int row = UIHelper::GetListViewSelectedRow(controls_[ID_LIST_VIEW]);
    if (row == -1) {
        UIHelper::ShowMessageBox(hWnd_, L"请先选择一个字段", L"提示");
        return;
    }
    
    int64_t id = (int64_t)UIHelper::GetListViewItemData(controls_[ID_LIST_VIEW], row);
    
    if (DeviceRepository::Instance().MoveParamField(id, true)) {
        LoadData();
    }
}

void ParamSettingsPage::OnMoveDown() {
    int row = UIHelper::GetListViewSelectedRow(controls_[ID_LIST_VIEW]);
    if (row == -1) {
        UIHelper::ShowMessageBox(hWnd_, L"请先选择一个字段", L"提示");
        return;
    }
    
    int64_t id = (int64_t)UIHelper::GetListViewItemData(controls_[ID_LIST_VIEW], row);
    
    if (DeviceRepository::Instance().MoveParamField(id, false)) {
        LoadData();
    }
}

void ParamSettingsPage::OnClose() {
    SendMessage(hParent_, WM_COMMAND, 12, 0);
}
