/** @file
 * @brief Own immutable raw input, exact edit metadata and counted allocation failures per case.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#ifndef AMA_SOURCE_EDITS_TEST_COMMON
#define AMA_SOURCE_EDITS_TEST_COMMON
#include <stdint.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/edits/edits.h"
static SourceTextEdits edits;
static const SourceText *input;
static const unsigned char raw[] = {'a', 'b', 'c', 'd', 0, 255, '\r', '\n'};
/** Borrow exact input bytes through edit teardown, without parsing or filesystem reads. */
static void prepare_bytes(const unsigned char *bytes, size_t length) {
    setup_test(); assert(source_manager_add(&sources, "original", bytes, length, &input) == SOURCE_OK);
    source_text_edits_init(&edits, sources.allocator, input);
}
/** Keep raw NUL/non-UTF8/CRLF bytes available for exact unaffected-segment assertions. */
static void prepare(void) { prepare_bytes(raw, sizeof(raw)); }
/** The counted allocator must return to zero after all borrowers retire. */
static void finish(void) { source_text_edits_destroy(&edits); destroy_test(); }
/** One independent replacement/deletion/boundary-insertion model used by apply and provenance cases. */
static void model_edit(unsigned selected) {
    static const unsigned char replacement[] = {'X', 'Y', 'Z'};
    static const unsigned char before[] = {'<'}, after[] = {'!'};
    const SourceSpan spans[] = {{input, 1, 3}, {input, 4, 6}, {input, 1, 1}, {input, 3, 3}};
    const unsigned char *bytes[] = {replacement, NULL, before, after};
    const size_t lengths[] = {3, 0, 1, 1};
    assert(source_text_edits_add(&edits, spans[selected], bytes[selected], lengths[selected]) == SOURCE_EDITS_OK);
}
#endif
