/** @file
 * @brief Reuse captured native companion authority under projected diagnostic paths.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * validates before destination creation; native bodies remain opaque and unexecuted.
 */
#include <string.h>
#include "../internal.h"
#include "../../internal.h"
/** The original project has already activated every required native module; no future path is probed. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_native(RelocationStaging *staging, const RelocationStagedSource *source, const ModuleSource *final) {
    const ModuleSource *original = NULL;
    for (const ProjectModule *module = staging->references->project.first; module != NULL; module = module->next) {
        if (strcmp(source_path(module->source->text),source->module->before_path) == 0) {
            original = module->source;
            break;
        }
    }
    if (original == NULL || !original->native_loaded) { return true; }
    const RelocationPreview *preview = staging->references->preview;
    const char *body = original->native_c == NULL ? NULL
        : relocation_project_to(&staging->arena,preview,original->native_c);
    const char *header = original->native_header == NULL ? NULL
        : relocation_project_to(&staging->arena,preview,original->native_header);
    const SourceText *abi = NULL;
    if ((original->native_c != NULL && body == NULL) || (original->native_header != NULL && header == NULL)) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Projected native path allocation failed",(SourceSpan){0});
    }
    if (original->native_abi != NULL) {
        const char *path = relocation_project_to(&staging->arena,preview,source_path(original->native_abi));
        if (path == NULL || source_manager_add(&staging->sources,path,source_bytes(original->native_abi),
            source_length(original->native_abi),&abi) != SOURCE_OK) {
            return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Projected native ABI allocation failed",(SourceSpan){0});
        }
    }
    ModuleStatus status = module_catalog_snapshot_native(&staging->project.catalog,final,body,header,abi);
    if (status != MODULE_OK) {
        return relocation_staging_fail(staging,status == MODULE_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY
            : RELOCATION_INVALID_SOURCE,"Captured native companions could not be installed",(SourceSpan){0});
    }
    return true;
}
