/** @file
 * @brief Project associated tests through the deepest original directory mapping.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * assigns nested associated tests to their most deeply containing source
 * association. Physical placement and overlapping file-map safety remain separate.
 */
#ifndef AMA_PROJECT_SOURCE_TEST_MAPPING_H
#define AMA_PROJECT_SOURCE_TEST_MAPPING_H
#include "source/memory/arena.h"
/** Borrow canonical logical test directory namespaces derived from source
 * associations, never physical plus-directory names or arbitrary raw prefixes.
 */
typedef struct { const char *original, *destination; } SourceTestDirectoryMap;
/** No associated directory is a distinct successful search outcome. */
typedef enum { SOURCE_TEST_MAP_OK, SOURCE_TEST_MAP_OUTSIDE,
    SOURCE_TEST_MAP_INVALID, SOURCE_TEST_MAP_CONFLICT, SOURCE_TEST_MAP_MEMORY } SourceTestMapStatus;
/** Project one complete canonical test identity through explicit directory maps.
 * Validate every pair before selection; duplicate original directories must agree.
 * Match strict descendants on slash boundaries, choose the longest original
 * prefix, and preserve the complete remaining suffix. A main with the same name
 * as a directory is outside that directory. Map order cannot affect the result.
 * On OK output is arena-owned; otherwise output stays unchanged. No I/O, source
 * mutation, physical/catalog admission or fallback for unmatched helpers occurs.
 * O(pair-count squared * path bytes) time, O(result bytes) owned storage.
 */
SourceTestMapStatus source_associated_test_identity(SourceArena *,
    const SourceTestDirectoryMap *, size_t, const char *identity, const char **output);
#endif
