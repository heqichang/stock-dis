#include "UIHelper.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

namespace UIHelper {

void InitCommonControls() {
    INITCOMMONCONTROLSEX icex = {};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);
}

HWND CreateButton(HWND hParent, int id, const std::wstring& text, int x, int y, int width, int height) {
    return CreateWindow(L"BUTTON", text.c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                       x, y, width, height, hParent, (HMENU)id, GetModuleHandle(nullptr), nullptr);
}

HWND CreateEdit(HWND hParent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style) {
    HWND hWnd = CreateWindow(L"EDIT", text.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | style,
                            x, y, width, height, hParent, (HMENU)id, GetModuleHandle(nullptr), nullptr);
    return hWnd;
}

HWND CreateStatic(HWND hParent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style) {
    return CreateWindow(L"STATIC", text.c_str(), WS_CHILD | WS_VISIBLE | style,
                       x, y, width, height, hParent, (HMENU)id, GetModuleHandle(nullptr), nullptr);
}

HWND CreateComboBox(HWND hParent, int id, int x, int y, int width, int height) {
    return CreateWindow(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                       x, y, width, height, hParent, (HMENU)id, GetModuleHandle(nullptr), nullptr);
}

HWND CreateListView(HWND hParent, int id, int x, int y, int width, int height) {
    HWND hWnd = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                            x, y, width, height, hParent, (HMENU)id, GetModuleHandle(nullptr), nullptr);
    
    if (hWnd) {
        ListView_SetExtendedListViewStyle(hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
    }
    
    return hWnd;
}

HWND CreateGroupBox(HWND hParent, int id, const std::wstring& text, int x, int y, int width, int height) {
    return CreateWindow(L"BUTTON", text.c_str(), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                       x, y, width, height, hParent, (HMENU)id, GetModuleHandle(nullptr), nullptr);
}

void SetEditText(HWND hWnd, const std::wstring& text) {
    SetWindowText(hWnd, text.c_str());
}

std::wstring GetEditText(HWND hWnd) {
    int length = GetWindowTextLength(hWnd);
    std::wstring text(length + 1, 0);
    GetWindowText(hWnd, &text[0], length + 1);
    text.resize(length);
    return text;
}

void SetComboBoxItem(HWND hWnd, const std::wstring& text, int data) {
    int index = ComboBox_AddString(hWnd, text.c_str());
    ComboBox_SetItemData(hWnd, index, data);
}

int GetComboBoxSelectedData(HWND hWnd) {
    int index = ComboBox_GetCurSel(hWnd);
    if (index == CB_ERR) return -1;
    return (int)ComboBox_GetItemData(hWnd, index);
}

void AddListViewColumn(HWND hListView, int index, const std::wstring& text, int width) {
    LVCOLUMN lvc = {};
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.iSubItem = index;
    lvc.pszText = const_cast<wchar_t*>(text.c_str());
    lvc.cx = width;
    lvc.fmt = LVCFMT_LEFT;
    ListView_InsertColumn(hListView, index, &lvc);
}

void AddListViewItem(HWND hListView, int row, int col, const std::wstring& text, LPARAM lParam) {
    LVITEM lvi = {};
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.iItem = row;
    lvi.iSubItem = col;
    lvi.pszText = const_cast<wchar_t*>(text.c_str());
    lvi.lParam = lParam;
    
    if (col == 0) {
        ListView_InsertItem(hListView, &lvi);
    } else {
        ListView_SetItem(hListView, &lvi);
    }
}

void SetListViewItem(HWND hListView, int row, int col, const std::wstring& text) {
    LVITEM lvi = {};
    lvi.mask = LVIF_TEXT;
    lvi.iItem = row;
    lvi.iSubItem = col;
    lvi.pszText = const_cast<wchar_t*>(text.c_str());
    ListView_SetItem(hListView, &lvi);
}

void ClearListView(HWND hListView) {
    ListView_DeleteAllItems(hListView);
}

int GetListViewSelectedRow(HWND hListView) {
    return ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
}

std::vector<int> GetListViewSelectedRows(HWND hListView) {
    std::vector<int> rows;
    int row = -1;
    while ((row = ListView_GetNextItem(hListView, row, LVNI_SELECTED)) != -1) {
        rows.push_back(row);
    }
    return rows;
}

LPARAM GetListViewItemData(HWND hListView, int row) {
    LVITEM lvi = {};
    lvi.mask = LVIF_PARAM;
    lvi.iItem = row;
    lvi.iSubItem = 0;
    ListView_GetItem(hListView, &lvi);
    return lvi.lParam;
}

int ShowMessageBox(HWND hParent, const std::wstring& text, const std::wstring& caption, UINT type) {
    return MessageBox(hParent, text.c_str(), caption.c_str(), type);
}

bool ShowConfirmDialog(HWND hParent, const std::wstring& text, const std::wstring& caption) {
    return MessageBox(hParent, text.c_str(), caption.c_str(), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES;
}

}
