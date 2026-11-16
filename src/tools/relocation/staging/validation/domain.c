/** @file
 * @brief Independently parse and type every final active source from prepared bytes.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects invalid staged projects before applying any change.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../internal.h"
#include "diagnostics/adapters/adapters.h"
/** Preserve deterministic required-module order independently of physical/root order. */
static int compare_names(const void *left, const void *right) {
    return strcmp(*(const char *const *)left,*(const char *const *)right);
}
/** Build a final unique registry from the actual prepared bytes, retaining drafts as inactive candidates. */
static bool snapshot(RelocationStaging *staging, const char *const **required, size_t *required_count) {
    size_t count = 0;
    for (const RelocationStagedSource *source = staging->first; source != NULL; source = source->next) { count++; }
    if (count > SIZE_MAX/sizeof(ModuleSnapshotEntry)) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Prepared registry is too large",(SourceSpan){0});
    }
    ModuleSnapshotEntry *entries = source_arena_allocate(&staging->arena,count*sizeof(*entries));
    const char **active = source_arena_allocate(&staging->arena,count*sizeof(*active));
    if (entries == NULL || active == NULL) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Prepared registry allocation failed",(SourceSpan){0});
    }
    size_t index = 0, activated = 0;
    for (const RelocationStagedSource *source = staging->first; source != NULL; source = source->next) {
        const RelocationModule *module = source->module;
        entries[index++] = (ModuleSnapshotEntry){module->after_identity,source->text,module->device,module->inode};
        if (module->active) { active[activated++] = module->after_identity; }
    }
    ModuleStatus status = module_catalog_snapshot(&staging->project.catalog,entries,count);
    if (status != MODULE_OK) {
        staging->status = status == MODULE_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY : RELOCATION_INVALID_SOURCE;
        (void)diagnostic_from_module(&staging->project.catalog.issue,&staging->related,&staging->error);
        return false;
    }
    for (const RelocationStagedSource *source = staging->first; source != NULL; source = source->next) {
        const ModuleSource *final = NULL;
        if (module_catalog_find(&staging->project.catalog,source->module->after_identity,&final) != MODULE_OK) {
            return relocation_staging_fail(staging,RELOCATION_INVALID_SOURCE,"A prepared source has no final catalog record",(SourceSpan){0});
        }
        if (!relocation_staging_native(staging,source,final)) { return false; }
    }
    qsort(active,activated,sizeof(*active),compare_names);
    *required = active;
    *required_count = activated;
    return true;
}
/** Ordinary project and semantic checks operate on the final namespace without a synthetic executable root. */
bool relocation_staging_domain(RelocationStaging *staging) {
    const char *const *required = NULL;
    size_t count = 0;
    if (!snapshot(staging,&required,&count)) { return false; }
    ProjectStatus loaded = project_load_catalog_set(&staging->project,required,count);
    if (loaded != PROJECT_OK) {
        staging->status = loaded == PROJECT_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY : RELOCATION_INVALID_SOURCE;
        staging->error = staging->project.error;
        return false;
    }
    semantic_program_init(&staging->semantic,&staging->project,staging->arena.allocator);
    staging->semantic_initialized = true;
    SemanticStatus status = semantic_analyze(&staging->semantic);
    if (status != SEMANTIC_OK) {
        staging->status = status == SEMANTIC_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY : RELOCATION_INVALID_SOURCE;
        staging->error = staging->semantic.error;
        return false;
    }
    return true;
}
