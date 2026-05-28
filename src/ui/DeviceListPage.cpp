#include "DeviceListPage.h"
#include "ui/UIHelper.h"
#include "db/DeviceRepository.h"
#include "print/PrintService.h"
#include "barcode/BarcodeGenerator.h"
#include <sstream>

DeviceListPage::DeviceListPage() {}

void DeviceListPage::OnCreate() {
    UIHelper::CreateStatic(hWnd_, -1, L"搜索:", 10, 10, 40, 25);
    controls_[ID_SEARCH_EDIT] = UIHelper::CreateEdit(hWnd_, ID_SEARCH_EDIT, L"", 55, 10, 200, 25);
    controls_[ID_SEARCH_BUTTON] = UIHelper::CreateButton(hWnd_, ID_SEARCH_BUTTON, L"搜索", 265, 10, 80, 25);
    
    UIHelper::CreateStatic(hWnd_, -1, L"状态:", 355, 10, 40, 25);
    controls_[ID_STATUS_COMBO] = UIHelper::CreateComboBox(hWnd_, ID_STATUS_COMBO, 400, 10, 120, 100);
    UIHelper::SetComboBoxItem(controls_[ID_STATUS_COMBO], L"全部", -1);
    UIHelper::SetComboBoxItem(controls_[ID_STATUS_COMBO], L"使用中", 0);
    UIHelper::SetComboBoxItem(controls_[ID_STATUS_COMBO], L"报废", 1);
    SendMessage(controls_[ID_STATUS_COMBO], CB_SETCURSEL, 0, 0);
    
    controls_[ID_ADD_BUTTON] = UIHelper::CreateButton(hWnd_, ID_ADD_BUTTON, L"新增设备", 540, 10, 100, 35);
    controls_[ID_EDIT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_EDIT_BUTTON, L"编辑", 650, 10, 80, 35);
    controls_[ID_DELETE_BUTTON] = UIHelper::CreateButton(hWnd_, ID_DELETE_BUTTON, L"删除", 740, 10, 80, 35);
    controls_[ID_PRINT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_PRINT_BUTTON, L"打印标签", 830, 10, 100, 35);
    controls_[ID_EXPORT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_EXPORT_BUTTON, L"导出", 940, 10, 80, 35);
    controls_[ID_IMPORT_BUTTON] = UIHelper::CreateButton(hWnd_, ID_IMPORT_BUTTON, L"导入", 1030, 10, 80, 35);
    
    controls_[ID_LIST_VIEW] = UIHelper::CreateListView(hWnd_, ID_LIST_VIEW, 10, 55, 1100, 500);
    
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 0, L"ID", 60);
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 1, L"设备名称", 250);
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 2, L"识别码", 180);
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 3, L"状态", 100);
    UIHelper::AddListViewColumn(controls_[ID_LIST_VIEW], 4, L"更新时间", 180);
    
    controls_[ID_PREV_PAGE] = UIHelper::CreateButton(hWnd_, ID_PREV_PAGE, L"上一页", 10, 565, 100, 30);
    controls_[ID_NEXT_PAGE] = UIHelper::CreateButton(hWnd_, ID_NEXT_PAGE, L"下一页", 1010, 565, 100, 30);
    controls_[ID_PAGE_INFO] = UIHelper::CreateStatic(hWnd_, ID_PAGE_INFO, L"", 450, 565, 220, 30, SS_CENTER);
    
    LoadData();
}

void DeviceListPage::OnSize(int cx, int cy) {
    if (cx < 200 || cy < 200) return;
    
    int list_width = cx - 20;
    int list_height = cy - 110;
    
    SetWindowPos(controls_[ID_LIST_VIEW], nullptr, 10, 55, list_width, list_height, SWP_NOZORDER);
    
    int col_width = (list_width - 60 - 100 - 180 - 180);
    if (col_width < 150) col_width = 150;
    ListView_SetColumnWidth(controls_[ID_LIST_VIEW], 1, col_width);
    
    SetWindowPos(controls_[ID_PREV_PAGE], nullptr, 10, cy - 45, 100, 30, SWP_NOZORDER);
    SetWindowPos(controls_[ID_NEXT_PAGE], nullptr, cx - 110, cy - 45, 100, 30, SWP_NOZORDER);
    SetWindowPos(controls_[ID_PAGE_INFO], nullptr, (cx - 220) / 2, cy - 45, 220, 30, SWP_NOZORDER);
    
    int btn_y = cy - 45;
    SetWindowPos(controls_[ID_EXPORT_BUTTON], nullptr, cx - 300, btn_y, 80, 30, SWP_NOZORDER);
    SetWindowPos(controls_[ID_IMPORT_BUTTON], nullptr, cx - 210, btn_y, 80, 30, SWP_NOZORDER);
}

void DeviceListPage::OnCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
        case ID_SEARCH_BUTTON:
            OnSearch();
            break;
        case ID_SEARCH_EDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                OnSearch();
            }
            break;
        case ID_STATUS_COMBO:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                OnStatusFilter();
            }
            break;
        case ID_ADD_BUTTON:
            OnAdd();
            break;
        case ID_EDIT_BUTTON:
            OnEdit();
            break;
        case ID_DELETE_BUTTON:
            OnDelete();
            break;
        case ID_PRINT_BUTTON:
            OnPrint();
            break;
        case ID_EXPORT_BUTTON:
            OnExport();
            break;
        case ID_IMPORT_BUTTON:
            OnImport();
            break;
        case ID_PREV_PAGE:
            OnPrevPage();
            break;
        case ID_NEXT_PAGE:
            OnNextPage();
            break;
    }
}

LRESULT DeviceListPage::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_USER_REFRESH_LIST) {
        LoadData();
        return 0;
    }
    if (uMsg == WM_NOTIFY) {
        NMHDR* pNMHDR = (NMHDR*)lParam;
        if (pNMHDR->idFrom == ID_LIST_VIEW && pNMHDR->code == NM_DBLCLK) {
            OnEdit();
            return 0;
        }
    }
    return WindowBase::HandleMessage(uMsg, wParam, lParam);
}

void DeviceListPage::LoadData() {
    result_ = DeviceRepository::Instance().Query(query_);
    
    UIHelper::ClearListView(controls_[ID_LIST_VIEW]);
    
    for (size_t i = 0; i < result_.items.size(); ++i) {
        const auto& device = result_.items[i];
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 0, std::to_wstring(device.id), (LPARAM)device.id);
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 1, device.name);
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 2, device.code);
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 3, StatusToString(device.status));
        UIHelper::AddListViewItem(controls_[ID_LIST_VIEW], (int)i, 4, device.updated_at);
    }
    
    UpdatePagination();
}

void DeviceListPage::UpdatePagination() {
    std::wstringstream ss;
    ss << L"第 " << result_.page << L" / " << result_.page_count << L" 页，共 " << result_.total << L" 条记录";
    SetWindowText(controls_[ID_PAGE_INFO], ss.str().c_str());
    
    EnableWindow(controls_[ID_PREV_PAGE], result_.page > 1);
    EnableWindow(controls_[ID_NEXT_PAGE], result_.page < result_.page_count);
}

void DeviceListPage::RefreshData() {
    LoadData();
}

void DeviceListPage::OnSearch() {
    query_.keyword = UIHelper::GetEditText(controls_[ID_SEARCH_EDIT]);
    query_.page = 1;
    LoadData();
}

void DeviceListPage::OnStatusFilter() {
    int data = UIHelper::GetComboBoxSelectedData(controls_[ID_STATUS_COMBO]);
    if (data == -1) {
        query_.status = (DeviceStatus)-1;
    } else {
        query_.status = (DeviceStatus)data;
    }
    query_.page = 1;
    LoadData();
}

void DeviceListPage::OnAdd() {
    SendMessage(hParent_, WM_COMMAND, 1, 0);
}

void DeviceListPage::OnEdit() {
    int row = UIHelper::GetListViewSelectedRow(controls_[ID_LIST_VIEW]);
    if (row == -1) {
        UIHelper::ShowMessageBox(hWnd_, L"请先选择一条设备记录", L"提示");
        return;
    }
    int64_t id = (int64_t)UIHelper::GetListViewItemData(controls_[ID_LIST_VIEW], row);
    SendMessage(hParent_, WM_COMMAND, 2, (LPARAM)id);
}

void DeviceListPage::OnDelete() {
    int row = UIHelper::GetListViewSelectedRow(controls_[ID_LIST_VIEW]);
    if (row == -1) {
        UIHelper::ShowMessageBox(hWnd_, L"请先选择一条设备记录", L"提示");
        return;
    }
    
    if (!UIHelper::ShowConfirmDialog(hWnd_, L"确定要删除这条设备记录吗？此操作不可恢复。", L"确认删除")) {
        return;
    }
    
    int64_t id = (int64_t)UIHelper::GetListViewItemData(controls_[ID_LIST_VIEW], row);
    if (DeviceRepository::Instance().Delete(id)) {
        UIHelper::ShowMessageBox(hWnd_, L"删除成功", L"提示");
        LoadData();
    } else {
        UIHelper::ShowMessageBox(hWnd_, L"删除失败", L"错误", MB_OK | MB_ICONERROR);
    }
}

void DeviceListPage::OnPrint() {
    auto rows = UIHelper::GetListViewSelectedRows(controls_[ID_LIST_VIEW]);
    if (rows.empty()) {
        UIHelper::ShowMessageBox(hWnd_, L"请先选择要打印的设备记录", L"提示");
        return;
    }
    
    std::vector<PrintLabel> labels;
    for (int row : rows) {
        int64_t id = (int64_t)UIHelper::GetListViewItemData(controls_[ID_LIST_VIEW], row);
        auto device = DeviceRepository::Instance().GetById(id);
        if (device) {
            PrintLabel label;
            label.name = device->name;
            label.code = device->code;
            label.barcode = BarcodeGenerator::Instance().GenerateCode128(device->code);
            labels.push_back(label);
        }
    }
    
    if (!labels.empty()) {
        SendMessage(hParent_, WM_COMMAND, 3, (LPARAM)&labels);
    }
}

void DeviceListPage::OnExport() {
    SendMessage(hParent_, WM_COMMAND, 4, 0);
}

void DeviceListPage::OnImport() {
    SendMessage(hParent_, WM_COMMAND, 5, 0);
}

void DeviceListPage::OnPrevPage() {
    if (query_.page > 1) {
        query_.page--;
        LoadData();
    }
}

void DeviceListPage::OnNextPage() {
    if (query_.page < result_.page_count) {
        query_.page++;
        LoadData();
    }
}
