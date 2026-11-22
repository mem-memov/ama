/** @file
 * @brief Separate complete association preparation from raw test-member projection.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires deepest selection over all mapped associations before any test map.
 */
#ifndef AMA_SOURCE_COPY_TEST_MATERIAL_INTERNAL_H
#define AMA_SOURCE_COPY_TEST_MATERIAL_INTERNAL_H
#include "tests.h"
/** Arena-owned paths; clone borrows the immutable ready main relation. */
typedef struct SourceCopyTestAssociation {
    struct SourceCopyTestAssociation *next;
    const SourceCloneMapping *clone;
    const char *original, *destination;
} SourceCopyTestAssociation;
/** Validate the complete version table and collect unique actual test requesters.
 * Failed scratch stays owned by files; caller poisons sealing on refusal.
 */
SourceCopyTestsStatus source_copy_test_associations(SourceFileMap *, const SourceCopyClosure *,
    const SourceCloneMap *, const SourceVersionPlacement *, size_t,
    const SourceCopyTestAssociation **, SourceCopyTestsIssue *);
#endif
