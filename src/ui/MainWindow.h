#pragma once

#include "ui/WindowBase.h"
#include "core/common.h"
#include "print/PrintService.h"

class DeviceListPage;
class DeviceEditPage;
class DeviceDetailPage;
class PrintPreviewPage;

enum class PageType {
    LIST = 0,
    EDIT = 1,
    DETAIL = 2,
    PREVIEW = 3
};

class MainWindow : public WindowBase {
public:
    MainWindow();
    virtual ~MainWindow();
    
    bool CreateMainWindow();
    void SwitchPage(PageType page);
    void ShowDeviceDetail(int64_t deviceId);
    void ShowDeviceEdit(int64_t deviceId = 0);
    void ShowPrintPreview(const std::vector<PrintLabel>& labels);
    
protected:
    void OnCreate() override;
    void OnSize(int cx, int cy) override;
    void OnDestroy() override;
    void OnCommand(WPARAM wParam, LPARAM lParam) override;
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    
private:
    void OnScanCode(WPARAM wParam, LPARAM lParam);
    void ShowScanCodeDialog(const std::wstring& code);
    void OnExport();
    void OnImport();
    void ShowFileDialog(bool isExport, bool isJson);
    
    DeviceListPage* listPage_ = nullptr;
    DeviceEditPage* editPage_ = nullptr;
    DeviceDetailPage* detailPage_ = nullptr;
    PrintPreviewPage* previewPage_ = nullptr;
    PageType currentPage_ = PageType::LIST;
};
