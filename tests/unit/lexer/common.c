#ifndef AMA_LEXER_TEST_COMMON
#define AMA_LEXER_TEST_COMMON
#include "../source/common.c"
#include "lexer/layout/layout.h"

static SourceLayout layout;

static LayoutStatus read_layout(const char *text) {
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "source", (const unsigned char *)text,
        strlen(text), &source);
    assert(added == SOURCE_OK);
    layout_init(&layout, sources.allocator);
    return layout_read(&layout, source);
}

static void destroy_layout(void) {
    layout_destroy(&layout);
    destroy_test();
}
#endif
