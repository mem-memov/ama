#ifndef AMA_DIAGNOSTIC_TEST_COMMON
#define AMA_DIAGNOSTIC_TEST_COMMON

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "source/text/source.h"
#include "diagnostics/record/diagnostic.h"

static SourceManager sources;
static const SourceText *source;

static void setup_test(void) {
    source_manager_init(&sources, source_system_allocator());
    const unsigned char bytes[] = "@!\n商品\n.shop.item\n";
    SourceStatus status = source_manager_add(&sources, "商店/item", bytes, sizeof(bytes) - 1, &source);
    assert(status == SOURCE_OK);
}

static void destroy_test(void) {
    source_manager_destroy(&sources);
}

#endif
