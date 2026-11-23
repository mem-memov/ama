/** @file
 * @brief Admit one coherent original/copy/deletion input domain before image orchestration.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires exact original compiler inputs and explicit copy provenance;
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires an explicit mode for cross-version clones.
 */
#include "internal.h"
#include <string.h>
/** Check complete version boundaries, never version-like substrings in names. */
static bool mode(const SourceOperationPlanInputs *inputs) {
    if (!inputs->copy) { return true; }
    if (inputs->copy->mode != SOURCE_COPY_KEEP_EXTERNAL) { return true; }
    for (const SourceCloneMapping *row = inputs->mains->first; row; row = row->next) {
        const char *old = row->original->logical_path, *now = row->identity;
        ModulePathShape a, b;
        if (!module_path_describe((const unsigned char *)old, strlen(old), &a)
            || !module_path_describe((const unsigned char *)now, strlen(now), &b)) { return false; }
        if (a.version_length != b.version_length || memcmp(old, now, a.version_length)) { return false; }
    }
    return true;
}
SourceOperationPlanStatus source_operation_plan_admit(const SourceOperationPlanInputs *inputs) {
    if (!inputs || !inputs->original || !inputs->original->valid
        || !inputs->observed || !inputs->observed->valid || !inputs->observed->sealed
        || !inputs->mains || !inputs->mains->ready || inputs->mains->capture != inputs->observed->original
        || !inputs->files || !inputs->files->ready
        || !delivery_source_response_matches(&inputs->original->original.response, &inputs->observed->original->view)
        || (inputs->final_root_count && !inputs->final_roots) || (inputs->change_count && !inputs->changes)
        || (inputs->configuration_count && !inputs->configurations)) { return SOURCE_OPERATION_PLAN_INVALID; }
    if (inputs->copy) {
        if (!inputs->copy->valid || inputs->copy->tree->capture != inputs->observed->original
            || inputs->copy->expansion.analysis != inputs->original || !inputs->contexts || !inputs->contexts->ready
            || inputs->contexts->copy != inputs->copy || inputs->contexts->mains != inputs->mains) {
            return SOURCE_OPERATION_PLAN_INVALID;
        }
    } else if (!inputs->removal || inputs->contexts || inputs->texts || inputs->mains->first || inputs->files->first) {
        return SOURCE_OPERATION_PLAN_INVALID;
    }
    return mode(inputs) ? SOURCE_OPERATION_PLAN_OK : SOURCE_OPERATION_PLAN_MODE;
}
