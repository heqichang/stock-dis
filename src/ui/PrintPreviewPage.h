#pragma once

#include "ui/WindowBase.h"
#include "core/common.h"
#include "print/PrintService.h"

class PrintPreviewPage : public WindowBase {
public:
    PrintPreviewPage();
    
    void SetLabels(const std::vector<PrintLabel>& labels);
    
    enum ControlId {
        ID_BACK_BUTTON = 4001,
        ID_PRINT_BUTTON,
        ID_ZOOM_IN,
        ID_ZOOM_OUT,
        ID_PREV_PAGE,
        ID_NEXT_PAGE
    };
    
protected:
    void OnCreate() override;
    void OnPaint(HDC hdc) override;
    void OnSize(int cx, int cy) override;
    void OnCommand(WPARAM wParam, LPARAM lParam) override;
    
private:
    void CalculateLayout();
    void OnBack();
    void OnPrint();
    void OnZoomIn();
    void OnZoomOut();
    void OnPrevPage();
    void OnNextPage();
    
    std::vector<PrintLabel> labels_;
    int currentPage_ = 0;
    int totalPages_ = 0;
    int labelsPerPage_ = 0;
    int cols_ = 0;
    int rows_ = 0;
    float zoom_ = 1.0f;
    int labelWidth_ = 0;
    int labelHeight_ = 0;
    int marginX_ = 0;
    int marginY_ = 0;
};
