/** @file
 * @brief Share verified physical traversal, missing-prefix evidence and owned failure scratch.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) forbids guessed paths;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps scratch arena-owned.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * distinguishes strict source traversal from creation-only proposals;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * gives a missing-prefix record no permission to create or mutate source inputs.
 */
#ifndef AMA_SOURCE_PATH_PHYSICAL_INTERNAL_H
#define AMA_SOURCE_PATH_PHYSICAL_INTERNAL_H
#include "physical.h"
/** Publish a typed failure without allocating diagnostic scratch under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
static inline SourcePhysicalStatus source_physical_fail(SourcePhysicalIssue *issue,
    SourcePhysicalStatus status, const char *path, PathProbe probe) {
    if (issue) { *issue = (SourcePhysicalIssue){status, path, probe}; }
    return status;
}
/** Obtain the actual physical cwd with checked observed ERANGE growth under
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02).
 */
SourcePhysicalStatus source_physical_cwd(SourceArena *, const char **, SourcePhysicalIssue *);
/** First absent ordinary component and the untouched remaining input suffix.
 * Both strings belong to the walk arena. No frontier is produced for aliases,
 * wrong kinds or technical failures; it grants no creation authority under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct { const char *path, *remaining; } SourcePhysicalFrontier;
/** Shared exact prefix traversal. An optional frontier retains the first missing
 * component for explicit creation-destination normalization, without changing
 * strict source/missing-final semantics of
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01).
 * Output/frontier/issue are distinct. */
SourcePhysicalStatus source_physical_walk(SourceArena *, const char *, SourcePhysicalKind,
    bool missing_final, SourcePhysicalPath *, SourcePhysicalIssue *, SourcePhysicalFrontier *);
#endif
