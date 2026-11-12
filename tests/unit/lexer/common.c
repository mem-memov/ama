/**
 * @file
 * @brief Compose counted source storage with structural layout reading and reverse teardown.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_LEXER_TEST_COMMON
#define AMA_LEXER_TEST_COMMON
#include "../source/common.c"
#include "lexer/layout/layout.h"

static SourceLayout layout;

/**
 * Register borrowed input bytes in the owned source manager, initialize layout and return the
 * actual structural read status.
 */
static LayoutStatus read_layout(const char *text) {
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "source", (const unsigned char *)text,
        strlen(text), &source);
    assert(added == SOURCE_OK);
    layout_init(&layout, sources.allocator);
    return layout_read(&layout, source);
}

/**
 * Destroy layout records before their backing source manager and counted allocator fixture.
 */
static void destroy_layout(void) {
    layout_destroy(&layout);
    destroy_test();
}
#endif
