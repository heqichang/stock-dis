#ifndef ZINT_H
#define ZINT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BARCODE_CODE128 20

#define BARCODE_BOX       0x00080000

struct zint_symbol {
    int symbology;
    int height;
    int scale;
    int whitespace_width;
    int border_width;
    int output_options;
    char text[128];
    int bitmap_width;
    int bitmap_height;
    unsigned char* bitmap;
    void* debug;
};

typedef struct zint_symbol zint_symbol;

zint_symbol* ZBarcode_Create(void);
void ZBarcode_Delete(zint_symbol* symbol);
int ZBarcode_Encode(zint_symbol* symbol, const unsigned char* data, int length);

#ifdef __cplusplus
}
#endif

#endif
