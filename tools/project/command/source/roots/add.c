/** @file
 * @brief Join one exact final root setting while preserving kind and explicit scope provenance.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * permits explicit destination settings, never silent root-family changes.
 */
#include "internal.h"
#include <string.h>
ProjectSourceRootSettingsStatus project_source_root_settings_add(ProjectSourceRootSettings *owner,
    ModuleRoot root, bool explicit_choice, size_t *index) {
    for (size_t i = 0; i < owner->final_count; i++) {
        if (strcmp(owner->final[i].path, root.path)) { continue; }
        *index = i;
        if (owner->final[i].kind != root.kind) { owner->failed_path = root.path; return PROJECT_SOURCE_ROOTS_KIND; }
        if (explicit_choice) { owner->final[i] = root; }
        return PROJECT_SOURCE_ROOTS_OK;
    }
    if (owner->final_count == owner->capacity) { owner->failed_path = root.path; return PROJECT_SOURCE_ROOTS_LIMIT; }
    *index = owner->final_count; owner->final[owner->final_count++] = root; return PROJECT_SOURCE_ROOTS_OK;
}
