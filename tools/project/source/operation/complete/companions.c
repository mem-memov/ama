/** @file
 * @brief Require alias-preserving typed recipes for every active copied module configuration.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves even unused alias meanings; a matching final call graph cannot prove
 * an unused relative alias was correctly rebased. Draft bytes remain opaque.
 */
#include "internal.h"
#include <string.h>
SourceOperationCompleteStatus source_operation_complete_companions(SourceOperationComplete *owner) {
    const SourceOperationWritesInputs *inputs = &owner->writes.inputs;
    for (const SourceCloneMapping *clone = inputs->mains->first; clone; clone = clone->next) {
        if (!clone->original->configuration) { continue; }
        const DeliverySourceDefinition *active = owner->inputs.original->original.response.definitions;
        while (active && strcmp(active->identity, clone->original->logical_path)) { active = active->next; }
        if (!active) { continue; }
        bool present = false;
        for (size_t i = 0; i < inputs->module_count; i++) {
            if (inputs->modules[i]->inputs.clone == clone) { present = true; break; }
        }
        if (!present) { owner->failed_path = source_path(clone->original->configuration); return SOURCE_OPERATION_COMPLETE_CONFIGURATION; }
    }
    return SOURCE_OPERATION_COMPLETE_OK;
}
