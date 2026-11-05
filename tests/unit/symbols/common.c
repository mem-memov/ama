#ifndef AMA_TESTS_SYMBOLS_COMMON_C
#define AMA_TESTS_SYMBOLS_COMMON_C
#include "../common.c"
#include "symbols/table.h"
static SymbolTable* symbols;
static void setup_test(void) {
    test_count++;
    error_clear(&error);
    symbols = calloc(1, sizeof(*symbols));
    assert(symbols != NULL);
    symbols_init(symbols);
}
static void destroy_test(void) {
    free(symbols);
    symbols = NULL;
}
#endif
