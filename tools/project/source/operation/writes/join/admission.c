/** @file
 * @brief Admit exact common copy/delete facts before constructing a closed mutation set.
 * [TOOL-03](../../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * keeps deletion distinct from cloning; [TOOL-04](../../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids cyclic preparation input and unrelated producer ownership.
 */
#include "internal.h"
/** A finite singly linked input can be checked without allocation or iteration caps. */
static bool finite(const SourceGraphPreparedText *rows) {
    const SourceGraphPreparedText *slow = rows, *fast = rows;
    while (fast && fast->next) {
        slow = slow->next; fast = fast->next->next;
        if (slow == fast) { return false; }
    }
    return true;
}
SourceOperationWritesStatus source_operation_writes_admit(const SourceOperationWritesInputs *inputs) {
    if (!inputs || !inputs->observed || !inputs->observed->valid || !inputs->observed->sealed
        || !inputs->mains || !inputs->mains->ready || inputs->mains->capture != inputs->observed->original
        || !inputs->files || !inputs->files->ready || !finite(inputs->main_texts)
        || (inputs->initialization_count && !inputs->initializations)
        || (inputs->configuration_count && !inputs->configurations) || (inputs->module_count && !inputs->modules)) {
        return SOURCE_OPERATION_WRITES_INVALID;
    }
    if (inputs->copy) {
        if (!inputs->copy->valid || inputs->copy->tree->capture != inputs->observed->original) { return SOURCE_OPERATION_WRITES_INVALID; }
    } else if (!inputs->removal || inputs->mains->first || inputs->files->first || inputs->main_texts
        || inputs->initialization_count || inputs->configuration_count || inputs->module_count) { return SOURCE_OPERATION_WRITES_INVALID; }
    if (inputs->removal && (!inputs->removal->valid || inputs->removal->tree->capture != inputs->observed->original
        || (inputs->copy && inputs->removal->analysis != inputs->copy->expansion.analysis))) { return SOURCE_OPERATION_WRITES_INVALID; }
    return SOURCE_OPERATION_WRITES_OK;
}
