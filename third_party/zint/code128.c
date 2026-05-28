#include "zint.h"

int code128(zint_symbol* symbol, const unsigned char* source, int length) {
    return ZBarcode_Encode(symbol, source, length);
}
