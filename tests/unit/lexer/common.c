#ifndef AMA_TESTS_LEXER_COMMON_C
#define AMA_TESTS_LEXER_COMMON_C
#include "../common.c"
#include "lexer/lexer.h"
#include "lexer/sections.h"
static FILE* source;
static Lexer lexer;
static void setup_test(void) {
    count++;
    error_clear(&error);
    source = NULL;
}
static inline void set_source(const char* text) {
    source = text_file(text);
    bool initialized = init_lexer(&lexer, source, 0, (long)strlen(text), 1, &error);
    assert(initialized);
}
static void destroy_test(void) {
    if (source) close_file(source);
    source = NULL;
}
#endif
