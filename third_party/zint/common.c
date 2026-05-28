#include "zint.h"
#include <stdlib.h>
#include <string.h>

void error_tag(zint_symbol* symbol, const char* error_message) {
    if (symbol) {
        strncpy(symbol->text, error_message, sizeof(symbol->text) - 1);
    }
}

int is_sane(int utf8[], int size) {
    for (int i = 0; i < size; i++) {
        if (utf8[i] == 0) return 0;
    }
    return 1;
}
