#include "xlsxwriter.h"
#include <stdlib.h>
#include <string.h>

struct lxw_workbook {
    char filename[256];
};

struct lxw_worksheet {
    int dummy;
};

struct lxw_format {
    int bold;
    uint32_t bg_color;
    uint32_t font_color;
};

lxw_workbook* workbook_new(const char* filename) {
    lxw_workbook* wb = (lxw_workbook*)malloc(sizeof(lxw_workbook));
    if (!wb) return NULL;
    memset(wb, 0, sizeof(lxw_workbook));
    if (filename) {
        strncpy(wb->filename, filename, sizeof(wb->filename) - 1);
    }
    return wb;
}

lxw_error workbook_close(lxw_workbook* workbook) {
    if (!workbook) return LXW_ERROR_NULL_PARAMETER;
    free(workbook);
    return LXW_NO_ERROR;
}

lxw_worksheet* workbook_add_worksheet(lxw_workbook* workbook, const char* name) {
    if (!workbook) return NULL;
    lxw_worksheet* ws = (lxw_worksheet*)malloc(sizeof(lxw_worksheet));
    if (!ws) return NULL;
    memset(ws, 0, sizeof(lxw_worksheet));
    return ws;
}

lxw_format* workbook_add_format(lxw_workbook* workbook) {
    if (!workbook) return NULL;
    lxw_format* fmt = (lxw_format*)malloc(sizeof(lxw_format));
    if (!fmt) return NULL;
    memset(fmt, 0, sizeof(lxw_format));
    return fmt;
}

void format_set_bold(lxw_format* format) {
    if (format) format->bold = 1;
}

void format_set_bg_color(lxw_format* format, uint32_t color) {
    if (format) format->bg_color = color;
}

void format_set_font_color(lxw_format* format, uint32_t color) {
    if (format) format->font_color = color;
}

void worksheet_write_string(lxw_worksheet* worksheet, int row, int col, const char* string, lxw_format* format) {
}

void worksheet_write_number(lxw_worksheet* worksheet, int row, int col, double number, lxw_format* format) {
}

void worksheet_set_column(lxw_worksheet* worksheet, int first_col, int last_col, double width, lxw_format* format) {
}
