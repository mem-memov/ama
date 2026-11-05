#ifndef AMA_TESTS_CODEGEN_COMMON_C
#define AMA_TESTS_CODEGEN_COMMON_C
#include "../common.c"
#include "codegen/emitter.h"
static SymbolTable* symbols;
static Emitter emitter;
static FILE* output;
static void setup_test(void) {
    count++;
    error_clear(&error);
    symbols = malloc(sizeof(*symbols));
    output = tmpfile();
    assert(symbols != NULL && output != NULL);
    symbols_init(symbols);
    emitter_init(&emitter, output, symbols);
}
static void destroy_test(void) { close_file(output); free(symbols); }
#endif
