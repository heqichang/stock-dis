#include "zint.h"
#include <stdlib.h>
#include <string.h>

zint_symbol* ZBarcode_Create(void) {
    zint_symbol* symbol = (zint_symbol*)malloc(sizeof(zint_symbol));
    if (!symbol) return NULL;
    memset(symbol, 0, sizeof(zint_symbol));
    symbol->scale = 2;
    symbol->height = 50;
    return symbol;
}

void ZBarcode_Delete(zint_symbol* symbol) {
    if (symbol) {
        if (symbol->bitmap) {
            free(symbol->bitmap);
        }
        free(symbol);
    }
}

int ZBarcode_Encode(zint_symbol* symbol, const unsigned char* data, int length) {
    if (!symbol || !data) return 1;
    
    if (length <= 0) {
        length = strlen((const char*)data);
    }
    
    symbol->bitmap_width = length * 10;
    symbol->bitmap_height = symbol->height * symbol->scale;
    
    symbol->bitmap = (unsigned char*)malloc(symbol->bitmap_width * symbol->bitmap_height);
    if (!symbol->bitmap) return 1;
    
    memset(symbol->bitmap, 255, symbol->bitmap_width * symbol->bitmap_height);
    
    for (int i = 0; i < length; ++i) {
        for (int bar = 0; bar < 8; ++bar) {
            if (data[i] & (1 << bar)) {
                int x = i * 10 + bar;
                for (int y = 0; y < symbol->bitmap_height; ++y) {
                    if (x < symbol->bitmap_width) {
                        symbol->bitmap[y * symbol->bitmap_width + x] = 0;
                    }
                }
            }
        }
    }
    
    return 0;
}
