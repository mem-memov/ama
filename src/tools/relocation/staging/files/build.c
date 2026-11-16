/** @file
 * @brief Capture the carried physical tree and external contributing sources with exact planned replacements.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * preserves opaque native/assets bytes; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains originals for later revalidation and recovery.
 */
#include <string.h>
#include "../internal.h"
#include "../../internal.h"

/** Associate prepared Ama bytes only with the exact original regular source and device/inode authority. Component contract: [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool sources(RelocationStaging *staging) {
    for (const RelocationStagedSource *source = staging->first; source != NULL; source = source->next) {
        const RelocationModule *module = source->module;
        RelocationStagedFile *file = relocation_staging_capture(staging,module->before_path,module->after_path,NULL);
        if (file == NULL) { return false; }
        if (!S_ISREG(file->metadata.st_mode) || file->metadata.st_dev != module->device
            || file->metadata.st_ino != module->inode || file->original == NULL
            || source_length(file->original) != source_length(module->text)
            || memcmp(source_bytes(file->original),source_bytes(module->text),source_length(module->text)) != 0) {
            return relocation_staging_fail(staging,RELOCATION_CHANGED_INPUT,"An Ama source changed after its original bindings were resolved",(SourceSpan){module->text,0,0});
        }
        file->replacement = source->text;
        file->protected_input = true;
    }
    return true;
}
/** Native files are copied as raw bytes; captured ABI bytes must still match those used by original semantic analysis. Component contract: [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool companion(RelocationStaging *staging, const char *path, const SourceText *abi) {
    if (path == NULL) { return true; }
    const char *after = relocation_project_to(&staging->arena,staging->references->preview,path);
    if (after == NULL) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Native companion placement allocation failed",(SourceSpan){0});
    }
    RelocationStagedFile *file = relocation_staging_capture(staging,path,after,NULL);
    if (file == NULL) { return false; }
    if (!S_ISREG(file->metadata.st_mode) || (abi != NULL
        && (source_length(file->original) != source_length(abi)
            || memcmp(source_bytes(file->original),source_bytes(abi),source_length(abi)) != 0))) {
        return relocation_staging_fail(staging,RELOCATION_CHANGED_INPUT,"A native companion changed after original analysis",(SourceSpan){abi,0,0});
    }
    file->protected_input = true;
    return true;
}
/** Finite authoritative inventories determine scope; no suffix search scans arbitrary C, assets or drafts for paths. Component contract: [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_files(RelocationStaging *staging) {
    for (const RelocationEntry *entry = staging->references->preview->entries; entry != NULL; entry = entry->next) {
        if (relocation_staging_capture(staging,entry->before,entry->after,&entry->metadata) == NULL) { return false; }
    }
    if (!sources(staging)) { return false; }
    for (const ProjectModule *module = staging->references->project.first; module != NULL; module = module->next) {
        const ModuleSource *source = module->source;
        if (!source->native_loaded) { continue; }
        if (!companion(staging,source->native_c,NULL) || !companion(staging,source->native_header,NULL)
            || !companion(staging,source->native_abi == NULL ? NULL : source_path(source->native_abi),source->native_abi)) { return false; }
    }
    return true;
}
