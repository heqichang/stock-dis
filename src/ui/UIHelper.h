#pragma once

#include "core/common.h"

namespace UIHelper {

HWND CreateButton(HWND hParent, int id, const std::wstring& text, int x, int y, int width, int height);
HWND CreateEdit(HWND hParent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style = 0);
HWND CreateStatic(HWND hParent, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style = 0);
HWND CreateComboBox(HWND hParent, int id, int x, int y, int width, int height);
HWND CreateListView(HWND hParent, int id, int x, int y, int width, int height);
HWND CreateGroupBox(HWND hParent, int id, const std::wstring& text, int x, int y, int width, int height);

void SetEditText(HWND hWnd, const std::wstring& text);
std::wstring GetEditText(HWND hWnd);
void SetComboBoxItem(HWND hWnd, const std::wstring& text, int data);
int GetComboBoxSelectedData(HWND hWnd);

void AddListViewColumn(HWND hListView, int index, const std::wstring& text, int width);
void AddListViewItem(HWND hListView, int row, int col, const std::wstring& text, LPARAM lParam = 0);
void SetListViewItem(HWND hListView, int row, int col, const std::wstring& text);
void ClearListView(HWND hListView);
int GetListViewSelectedRow(HWND hListView);
std::vector<int> GetListViewSelectedRows(HWND hListView);
LPARAM GetListViewItemData(HWND hListView, int row);

int ShowMessageBox(HWND hParent, const std::wstring& text, const std::wstring& caption = L"提示", UINT type = MB_OK | MB_ICONINFORMATION);
bool ShowConfirmDialog(HWND hParent, const std::wstring& text, const std::wstring& caption = L"确认");

void InitCommonControls();

}
