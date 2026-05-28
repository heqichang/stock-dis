#pragma once

#include "ui/WindowBase.h"
#include "core/common.h"
#include <commctrl.h>

class DeviceListPage : public WindowBase {
public:
    DeviceListPage();
    
    enum ControlId {
        ID_SEARCH_EDIT = 1001,
        ID_SEARCH_BUTTON,
        ID_STATUS_COMBO,
        ID_ADD_BUTTON,
        ID_EDIT_BUTTON,
        ID_DELETE_BUTTON,
        ID_PRINT_BUTTON,
        ID_EXPORT_BUTTON,
        ID_IMPORT_BUTTON,
        ID_LIST_VIEW,
        ID_PREV_PAGE,
        ID_NEXT_PAGE,
        ID_PAGE_INFO
    };
    
    void RefreshData();
    
protected:
    void OnCreate() override;
    void OnSize(int cx, int cy) override;
    void OnCommand(WPARAM wParam, LPARAM lParam) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    
private:
    void LoadData();
    void UpdatePagination();
    void OnSearch();
    void OnAdd();
    void OnEdit();
    void OnDelete();
    void OnPrint();
    void OnExport();
    void OnImport();
    void OnPrevPage();
    void OnNextPage();
    void OnStatusFilter();
    
    DeviceQuery query_;
    PagedResult result_;
};
