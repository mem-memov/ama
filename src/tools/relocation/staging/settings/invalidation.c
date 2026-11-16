/** @file
 * @brief Queue only explicit generated outputs after rejecting source, settings and root overlap.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * invalidates generated artifacts rather than patching generated symbols;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * includes that effect in the later recoverable transaction.
 */
#include <string.h>
#include "../internal.h"
#include "../../internal.h"

/** An invalidation cannot consume either endpoint or an original/final configured source root. */
static bool contains_root(const RelocationPreview *preview, const char *before, const char *after) {
    if (relocation_contains(before,preview->source) || relocation_contains(after,preview->destination)) { return true; }
    for (size_t index = 0; index < preview->before_root_count; index++) {
        if (relocation_contains(before,preview->before_roots[index].path)) { return true; }
    }
    for (size_t index = 0; index < preview->after_root_count; index++) {
        if (relocation_contains(after,preview->after_roots[index].path)) { return true; }
    }
    return false;
}
/** Source and settings inputs take precedence over an invalid user declaration that labels them generated. */
bool relocation_staging_invalidation(RelocationStaging *staging, const char *input) {
    bool existed;
    const char *before = relocation_staging_setting_path(staging,input,true,&existed);
    if (before == NULL) { return false; }
    const RelocationPreview *preview = staging->references->preview;
    const char *after = relocation_project_to(&staging->arena,preview,before);
    if (after == NULL) { return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Generated output placement allocation failed",(SourceSpan){0}); }
    if (contains_root(preview,before,after)) {
        return relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"A generated output overlaps a source root or relocation endpoint",(SourceSpan){0});
    }
    struct stat metadata = {0};
    if (existed && lstat(before,&metadata) != 0) {
        return relocation_staging_fail(staging,RELOCATION_CHANGED_INPUT,"A generated output changed during inspection",(SourceSpan){0});
    }
    if (existed && !S_ISREG(metadata.st_mode) && !S_ISDIR(metadata.st_mode)) {
        return relocation_staging_fail(staging,RELOCATION_UNSUPPORTED_PATH,"A generated output must be a regular file or directory",(SourceSpan){0});
    }
    for (const RelocationStagedFile *file = staging->files; file != NULL; file = file->next) {
        if (file->protected_input && (relocation_contains(before,file->before) || relocation_contains(after,file->after))) {
            return relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"A generated output overlaps an original or final protected input",(SourceSpan){0});
        }
        if (existed && file->metadata.st_dev == metadata.st_dev && file->metadata.st_ino == metadata.st_ino
            && strcmp(file->before,before) != 0) {
            return relocation_staging_fail(staging,RELOCATION_ALIAS,"A generated output aliases a captured input",(SourceSpan){0});
        }
    }
    for (const RelocationInvalidation *other = staging->invalidations; other != NULL; other = other->next) {
        if (relocation_contains(before,other->before) || relocation_contains(other->before,before)
            || relocation_contains(after,other->after) || relocation_contains(other->after,after)) {
            return relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"Explicit generated outputs overlap",(SourceSpan){0});
        }
    }
    RelocationInvalidation *invalidation = source_arena_allocate(&staging->arena,sizeof(*invalidation));
    if (invalidation == NULL) { return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Generated output invalidation allocation failed",(SourceSpan){0}); }
    *invalidation = (RelocationInvalidation){.next=staging->invalidations,.before=before,.after=after,.metadata=metadata,.existed=existed};
    staging->invalidations = invalidation;
    return true;
}
