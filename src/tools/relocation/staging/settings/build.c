/** @file
 * @brief Prepare explicit entry/library settings and generated-output invalidations after final graph checks.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * returns changed external settings without inventing a project configuration syntax.
 */
#include <string.h>
#include "../internal.h"

/** Entry selection preserves the exact executable type under F, including newly covered sources. Component contract: [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01). */
static bool entry(RelocationStaging *staging, const char *name) {
    if (name == NULL) { return true; }
    const SemanticType *original = semantic_find_type(&staging->references->frontend.semantic,name);
    if (original == NULL || !original->executable) {
        return relocation_staging_fail(staging,RELOCATION_INVALID_REFERENCE,"The explicit entry does not name an original executable type",(SourceSpan){0});
    }
    for (const RelocationModule *module = staging->references->preview->modules; module != NULL; module = module->next) {
        if (strcmp(module->before_path,source_path(original->module->source->text)) == 0) {
            const SemanticType *final = semantic_find_type(&staging->frontend.semantic,module->after_identity);
            if (final == NULL || !final->executable) { break; }
            staging->entry = module->after_identity;
            return true;
        }
    }
    return relocation_staging_fail(staging,RELOCATION_INVALID_REFERENCE,"The explicit entry has no final executable mapping",(SourceSpan){0});
}
/** All configuration dependencies are protected before any requested invalidation is accepted. Component contract: [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01). */
bool relocation_staging_settings(RelocationStaging *staging, const RelocationStagingRequest *request) {
    if ((request->library_configuration_count != 0 && request->library_configurations == NULL)
        || (request->generated_output_count != 0 && request->generated_outputs == NULL)) {
        return relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"Explicit settings arrays are incomplete",(SourceSpan){0});
    }
    if (!entry(staging,request->entry)) { return false; }
    for (size_t index = 0; index < request->library_configuration_count; index++) {
        if (!relocation_staging_configuration(staging,request->library_configurations[index])) { return false; }
    }
    for (size_t index = 0; index < request->generated_output_count; index++) {
        if (!relocation_staging_invalidation(staging,request->generated_outputs[index])) { return false; }
    }
    return true;
}
