#include "MainWindow.h"
#include "ui/DeviceListPage.h"
#include "ui/DeviceEditPage.h"
#include "ui/DeviceDetailPage.h"
#include "ui/PrintPreviewPage.h"
#include "ui/ParamSettingsPage.h"
#include "ui/UIHelper.h"
#include "db/Database.h"
#include "db/DeviceRepository.h"
#include "utils/ScanCodeListener.h"
#include "utils/DataImportExport.h"
#include "print/PrintService.h"
#include "barcode/BarcodeGenerator.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <sstream>
#pragma comment(lib, "shell32.lib")

MainWindow::MainWindow() {}

MainWindow::~MainWindow() {
    ScanCodeListener::Instance().Stop();
    
    delete listPage_;
    delete editPage_;
    delete detailPage_;
    delete previewPage_;
    delete paramSettingsPage_;
}

bool MainWindow::CreateMainWindow() {
    std::wstring dbPath;
    wchar_t szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    PathRemoveFileSpec(szPath);
    dbPath = szPath;
    dbPath += L"\\devices.db";
    
    if (!Database::Instance().Init(dbPath)) {
        return false;
    }
    
    if (!Create(nullptr, L"公司设备管理系统", CW_USEDEFAULT, CW_USEDEFAULT, 1200, 700)) {
        return false;
    }
    
    ScanCodeListener::Instance().Start(hWnd_);
    
    return true;
}

void MainWindow::OnCreate() {
    UIHelper::InitFonts();
    
    listPage_ = new DeviceListPage();
    listPage_->Create(hWnd_, L"", 0, 0, 1200, 700);
    
    editPage_ = new DeviceEditPage();
    editPage_->Create(hWnd_, L"", 0, 0, 1200, 700);
    
    detailPage_ = new DeviceDetailPage();
    detailPage_->Create(hWnd_, L"", 0, 0, 1200, 700);
    
    previewPage_ = new PrintPreviewPage();
    previewPage_->Create(hWnd_, L"", 0, 0, 1200, 700);
    
    paramSettingsPage_ = new ParamSettingsPage();
    paramSettingsPage_->Create(hWnd_, L"", 0, 0, 1200, 700);
    
    SwitchPage(PageType::LIST);
}

void MainWindow::OnSize(int cx, int cy) {
    if (cx < 200 || cy < 200) return;
    
    if (listPage_) SetWindowPos(listPage_->GetHandle(), nullptr, 0, 0, cx, cy, SWP_NOZORDER);
    if (editPage_) SetWindowPos(editPage_->GetHandle(), nullptr, 0, 0, cx, cy, SWP_NOZORDER);
    if (detailPage_) SetWindowPos(detailPage_->GetHandle(), nullptr, 0, 0, cx, cy, SWP_NOZORDER);
    if (previewPage_) SetWindowPos(previewPage_->GetHandle(), nullptr, 0, 0, cx, cy, SWP_NOZORDER);
    if (paramSettingsPage_) SetWindowPos(paramSettingsPage_->GetHandle(), nullptr, 0, 0, cx, cy, SWP_NOZORDER);
}

void MainWindow::OnDestroy() {
    UIHelper::ReleaseFonts();
    Database::Instance().Close();
    PostQuitMessage(0);
}

void MainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
        case 1:
            ShowDeviceEdit(0);
            break;
        case 2:
            ShowDeviceEdit((int64_t)lParam);
            break;
        case 3: {
            std::vector<PrintLabel>* pLabels = (std::vector<PrintLabel>*)lParam;
            if (pLabels) {
                ShowPrintPreview(*pLabels);
            }
            break;
        }
        case 4:
            OnExport();
            break;
        case 5:
            OnImport();
            break;
        case 6:
            ShowParamSettings();
            break;
        case 10: {
            int64_t deviceId = (int64_t)lParam;
            if (deviceId > 0) {
                ShowDeviceDetail(deviceId);
            } else {
                SwitchPage(PageType::LIST);
            }
            listPage_->RefreshData();
            break;
        }
        case 11:
            SwitchPage(PageType::LIST);
            break;
        case 12:
            SwitchPage(PageType::LIST);
            listPage_->RefreshData();
            break;
        case 13:
            SwitchPage(PageType::LIST);
            break;
    }
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_USER_SCAN_CODE) {
        OnScanCode(wParam, lParam);
        return 0;
    }
    return WindowBase::HandleMessage(uMsg, wParam, lParam);
}

void MainWindow::SwitchPage(PageType page) {
    currentPage_ = page;
    
    if (listPage_) listPage_->Hide();
    if (editPage_) editPage_->Hide();
    if (detailPage_) detailPage_->Hide();
    if (previewPage_) previewPage_->Hide();
    if (paramSettingsPage_) paramSettingsPage_->Hide();
    
    switch (page) {
        case PageType::LIST:
            if (listPage_) {
                listPage_->Show();
                SetWindowText(hWnd_, L"公司设备管理系统 - 设备列表");
            }
            break;
        case PageType::EDIT:
            if (editPage_) {
                editPage_->Show();
                SetWindowText(hWnd_, L"公司设备管理系统 - 编辑设备");
            }
            break;
        case PageType::DETAIL:
            if (detailPage_) {
                detailPage_->Show();
                SetWindowText(hWnd_, L"公司设备管理系统 - 设备详情");
            }
            break;
        case PageType::PREVIEW:
            if (previewPage_) {
                previewPage_->Show();
                SetWindowText(hWnd_, L"公司设备管理系统 - 打印预览");
            }
            break;
        case PageType::PARAM_SETTINGS:
            if (paramSettingsPage_) {
                paramSettingsPage_->RefreshData();
                paramSettingsPage_->Show();
                SetWindowText(hWnd_, L"公司设备管理系统 - 参数设置");
            }
            break;
    }
}

void MainWindow::ShowDeviceDetail(int64_t deviceId) {
    if (detailPage_) {
        detailPage_->SetDevice(deviceId);
    }
    SwitchPage(PageType::DETAIL);
}

void MainWindow::ShowDeviceEdit(int64_t deviceId) {
    if (editPage_) {
        if (deviceId > 0) {
            editPage_->SetEditDevice(deviceId);
        } else {
            editPage_->Clear();
        }
    }
    SwitchPage(PageType::EDIT);
}

void MainWindow::ShowPrintPreview(const std::vector<PrintLabel>& labels) {
    if (previewPage_) {
        previewPage_->SetLabels(labels);
    }
    SwitchPage(PageType::PREVIEW);
}

void MainWindow::ShowParamSettings() {
    SwitchPage(PageType::PARAM_SETTINGS);
}

void MainWindow::OnScanCode(WPARAM wParam, LPARAM lParam) {
    std::wstring* pCode = (std::wstring*)lParam;
    if (!pCode) return;
    
    std::wstring code = *pCode;
    delete pCode;
    
    if (code.empty()) return;
    
    ShowScanCodeDialog(code);
}

void MainWindow::ShowScanCodeDialog(const std::wstring& code) {
    auto device = DeviceRepository::Instance().GetByCode(code);
    
    if (!device) {
        std::wstring msg = L"未找到识别码为 [" + code + L"] 的设备";
        UIHelper::ShowMessageBox(hWnd_, msg, L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    
    std::wstringstream ss;
    ss << L"设备名称: " << device->name << L"\r\n\r\n";
    ss << L"识别码: " << device->code << L"\r\n\r\n";
    ss << L"状态: " << StatusToString(device->status) << L"\r\n\r\n";
    ss << L"点击\"是\"查看详情，点击\"否\"打印标签";
    
    int ret = MessageBox(hWnd_, ss.str().c_str(), L"扫描到设备", MB_YESNOCANCEL | MB_ICONINFORMATION);
    
    if (ret == IDYES) {
        ShowDeviceDetail(device->id);
    } else if (ret == IDNO) {
        PrintLabel label;
        label.name = device->name;
        label.code = device->code;
        label.barcode = BarcodeGenerator::Instance().GenerateCode128(device->code);
        
        std::vector<PrintLabel> labels;
        labels.push_back(label);
        PrintService::Instance().PrintLabels(labels);
    }
}

void MainWindow::OnExport() {
    ShowFileDialog(true);
}

void MainWindow::OnImport() {
    ShowFileDialog(false);
}

void MainWindow::ShowFileDialog(bool isExport) {
    OPENFILENAME ofn = {};
    wchar_t szFileName[MAX_PATH] = {0};
    
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd_;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    
    ofn.lpstrFilter = L"Excel 文件 (*.xlsx)\0*.xlsx\0所有文件 (*.*)\0*.*\0";
    ofn.lpstrDefExt = L"xlsx";
    wcscpy_s(szFileName, L"设备数据.xlsx");
    
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (!isExport) {
        ofn.Flags |= OFN_FILEMUSTEXIST;
    }
    
    BOOL success = FALSE;
    if (isExport) {
        success = GetSaveFileName(&ofn);
    } else {
        success = GetOpenFileName(&ofn);
    }
    
    if (!success) return;
    
    std::wstring filePath = szFileName;
    
    if (isExport) {
        bool result = DataImportExport::Instance().ExportToExcel(filePath);
        if (result) {
            UIHelper::ShowMessageBox(hWnd_, L"导出成功", L"提示");
            listPage_->RefreshData();
        } else {
            UIHelper::ShowMessageBox(hWnd_, L"导出失败", L"错误", MB_OK | MB_ICONERROR);
        }
    } else {
        ImportResult result = DataImportExport::Instance().ImportFromExcel(filePath);
        std::wstringstream ss;
        ss << L"导入完成\r\n\r\n";
        ss << L"成功: " << result.success << L" 条\r\n";
        ss << L"跳过: " << result.skipped << L" 条\r\n";
        ss << L"失败: " << result.failed << L" 条";
        UIHelper::ShowMessageBox(hWnd_, ss.str(), L"导入结果");
        listPage_->RefreshData();
    }
}
