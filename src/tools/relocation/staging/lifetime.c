/** @file
 * @brief Retain destructible prepared prefixes and diagnostics without modifying original sources.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires failed preparation to leave the project untouched.
 */
#include "internal.h"
/** Each independent owner is initialized even when no build is attempted. */
void relocation_staging_init(RelocationStaging *staging, SourceAllocator allocator) {
    *staging = (RelocationStaging){0};
    source_arena_init(&staging->arena,allocator);
    source_manager_init(&staging->sources,allocator);
    project_init(&staging->project,allocator);
}
/** Release borrowers before their copied source and graph storage. */
void relocation_staging_destroy(RelocationStaging *staging) {
    for (RelocationStagedConfiguration *item = staging->configurations; item != NULL; item = item->next) {
        library_build_destroy(&item->after);
        library_build_destroy(&item->before);
    }
    if (staging->semantic_initialized) { semantic_program_destroy(&staging->semantic); }
    project_destroy(&staging->project);
    source_manager_destroy(&staging->sources);
    source_arena_destroy(&staging->arena);
    *staging = (RelocationStaging){0};
}
/** Error reporting cannot allocate and hide the primary preparation failure. */
bool relocation_staging_fail(RelocationStaging *staging, RelocationStatus status, const char *message, SourceSpan span) {
    if (staging->status == RELOCATION_OK) {
        staging->status = status;
        staging->error = (Diagnostic){.code=status == RELOCATION_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY
            : DIAGNOSTIC_INVALID_PATH,.message=message,.primary={span,NULL}};
    }
    return false;
}
