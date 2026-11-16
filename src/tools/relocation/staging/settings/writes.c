/** @file
 * @brief Preflight parent writes needed for external replacements and explicit output retirement.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * rejects unavailable required writes before any filesystem mutation.
 */
#include <string.h>
#include <unistd.h>
#include "../internal.h"
#include "../../internal.h"
/** Prepared replacements use sibling staging/rename, so parent write/search authority is required. Component contract: [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01). */
static bool parent(RelocationStaging *staging, const char *path) {
    char *directory = source_arena_copy(&staging->arena,path);
    if (directory == NULL) { return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Write preflight path allocation failed",(SourceSpan){0}); }
    char *slash = strrchr(directory,'/');
    if (slash == directory) { slash[1] = 0; } else { *slash = 0; }
    if (access(directory,W_OK|X_OK) == 0) { return true; }
    relocation_staging_fail(staging,RELOCATION_READ_ONLY,"An external replacement parent is not writable",(SourceSpan){0});
    staging->error.primary.path = directory;
    return false;
}
/** The carried tree is prepared independently; only external changed bytes and existing output paths need separate parent writes. Component contract: [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01). */
bool relocation_staging_writes(RelocationStaging *staging) {
    const RelocationPreview *preview = staging->references->preview;
    for (const RelocationStagedFile *file = staging->files; file != NULL; file = file->next) {
        if (relocation_contains(preview->source,file->before) || file->original == NULL || file->replacement == NULL) { continue; }
        bool changed = source_length(file->original) != source_length(file->replacement)
            || memcmp(source_bytes(file->original),source_bytes(file->replacement),source_length(file->original)) != 0;
        if (changed && !parent(staging,file->before)) { return false; }
    }
    for (const RelocationInvalidation *output = staging->invalidations; output != NULL; output = output->next) {
        if (output->existed && !relocation_contains(preview->source,output->before) && !parent(staging,output->before)) { return false; }
    }
    return true;
}
