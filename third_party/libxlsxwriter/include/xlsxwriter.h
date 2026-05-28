#ifndef XLSXWRITER_H
#define XLSXWRITER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lxw_workbook lxw_workbook;
typedef struct lxw_worksheet lxw_worksheet;
typedef struct lxw_format lxw_format;

typedef enum {
    LXW_NO_ERROR = 0,
    LXW_ERROR_NULL_PARAMETER,
    LXW_ERROR_CREATING_FILE,
    LXW_ERROR_MEMORY_ALLOCATION
} lxw_error;

#define LXW_COLOR_BLUE   0x0000FF
#define LXW_COLOR_WHITE  0xFFFFFF

lxw_workbook* workbook_new(const char* filename);
lxw_error workbook_close(lxw_workbook* workbook);

lxw_worksheet* workbook_add_worksheet(lxw_workbook* workbook, const char* name);
lxw_format* workbook_add_format(lxw_workbook* workbook);

void format_set_bold(lxw_format* format);
void format_set_bg_color(lxw_format* format, uint32_t color);
void format_set_font_color(lxw_format* format, uint32_t color);

void worksheet_write_string(lxw_worksheet* worksheet, int row, int col, const char* string, lxw_format* format);
void worksheet_write_number(lxw_worksheet* worksheet, int row, int col, double number, lxw_format* format);
void worksheet_set_column(lxw_worksheet* worksheet, int first_col, int last_col, double width, lxw_format* format);

#ifdef __cplusplus
}
#endif

#endif
