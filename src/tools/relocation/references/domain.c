/** @file
 * @brief Resolve the captured active domain with the ordinary project and semantic stages, without an executable entry.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * includes all scoped sources; [COMP-02](../../../../spec/implementation/compiler.md#comp-02)
 * forbids a second parser or guessed binding model.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "diagnostics/adapters/adapters.h"

/** New scope has no invented previous namespace: its explicit final-root identity names its analysis record. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
const char *relocation_analysis_name(const RelocationModule *module) {
    return module->before_identity == NULL ? module->after_identity : module->before_identity;
}
/** Required-set order is byte-exact and independent of physical directory enumeration or root order. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
static int compare_names(const void *left, const void *right) {
    return strcmp(*(const char *const *)left,*(const char *const *)right);
}

/** Copy all candidate records, retaining drafts for precise required-draft diagnostics but requiring active modules only. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
static bool snapshot(RelocationReferences *references, const char *const **required, size_t *required_count) {
    size_t count = 0;
    for (const RelocationModule *module = references->preview->modules; module != NULL; module = module->next) {
        if (count == SIZE_MAX/sizeof(ModuleSnapshotEntry)) {
            return relocation_references_fail(references,RELOCATION_OUT_OF_MEMORY,"The source snapshot is too large",(SourceSpan){0});
        }
        count++;
    }
    ModuleSnapshotEntry *entries = source_arena_allocate(&references->arena,count*sizeof(*entries));
    const char **active = source_arena_allocate(&references->arena,count*sizeof(*active));
    if (entries == NULL || active == NULL) {
        return relocation_references_fail(references,RELOCATION_OUT_OF_MEMORY,"Reference domain allocation failed",(SourceSpan){0});
    }
    size_t index = 0, activated = 0;
    for (const RelocationModule *module = references->preview->modules; module != NULL; module = module->next) {
        const char *name = relocation_analysis_name(module);
        entries[index++] = (ModuleSnapshotEntry){name,module->text,module->device,module->inode};
        if (module->active) { active[activated++] = name; }
    }
    ModuleStatus status = module_catalog_snapshot(&references->frontend.project.catalog,entries,count);
    if (status != MODULE_OK) {
        references->status = status == MODULE_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY : RELOCATION_INVALID_REFERENCE;
        (void)diagnostic_from_module(&references->frontend.project.catalog.issue,&references->related,&references->error);
        return false;
    }
    qsort(active,activated,sizeof(*active),compare_names);
    *required = active;
    *required_count = activated;
    return true;
}

/** Native companions are parsed through ordinary activation; no native body is executed or guessed from its filename. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [COMP-02](../../../../spec/implementation/compiler.md#comp-02). */
bool relocation_references_domain(RelocationReferences *references) {
    const char *const *required = NULL;
    size_t count = 0;
    if (!snapshot(references,&required,&count)) { return false; }
    if (!compiler_frontend_check_snapshot(&references->frontend,required,count)) {
        references->status = references->frontend.error.code == DIAGNOSTIC_OUT_OF_MEMORY
            ? RELOCATION_OUT_OF_MEMORY : RELOCATION_INVALID_REFERENCE;
        references->error = references->frontend.error;
        return false;
    }
    return true;
}
