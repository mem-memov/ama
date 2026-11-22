/** @file
 * @brief Share only physical traversal failure and actual working-directory preparation.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) forbids guessed paths;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps scratch arena-owned.
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
#endif
