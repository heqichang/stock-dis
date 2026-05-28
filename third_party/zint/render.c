#include "zint.h"
#include <stdlib.h>
#include <string.h>

int render_plot(zint_symbol* symbol, const char* text) {
    return ZBarcode_Encode(symbol, (const unsigned char*)text, 0);
}

int buffer_plot(zint_symbol* symbol) {
    return 0;
}
