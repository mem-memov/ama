/** @file
 * @brief Capture a finite exact input table without imposing stateful-export coverage rules.
 * [CLI-02](../../../spec/implementation/launch-and-embedding.md#cli-02)
 * admits ordinary defaults and owned scalar literal construction;
 * [ARCH-05](../../../spec/implementation/compiler.md#arch-05) bounds contiguous
 * record allocation before publication.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) keeps a refused
 * partial table under the same arena lifetime as successful preparation.
 */
#include <stddef.h>
#include <stdint.h>
#include "inputs.h"
/** One aligned allocation owns the immutable table and all entry descriptors.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) gives failed and
 * complete storage the same arena lifetime; declarations remain borrowed.
 */
typedef struct { DeliveryInputs table; DeliveryInput entries[]; } InputStorage;

bool delivery_inputs_prepare(SourceArena *arena, const DeliverySelection *selection,
    const DeliveryInputs **output, Diagnostic *error) {
    if (!arena || !selection || !output || !error) { return false; }
    const CompilerFrontend *frontend = selection->frontend;
    if (!frontend || !frontend->valid || frontend->authoring || frontend->plan.entry != selection->root || !selection->root) {
        *error = (Diagnostic){.code = DIAGNOSTIC_CHANGED_INPUT, .message = "Delivery input metadata no longer names this valid frontend entry"};
        return false;
    }
    const SemanticType *root = selection->root->type;
    size_t count = 0;
    for (const SemanticSymbol *field = root->first; field; field = field->next) {
        if (field->role != SYMBOL_PARAMETER) { continue; }
        if (count == SIZE_MAX) { goto no_memory; }
        count++;
    }
    size_t header = offsetof(InputStorage, entries);
    if (count > (SIZE_MAX - header) / sizeof(DeliveryInput)) { goto no_memory; }
    size_t bytes = header + count * sizeof(DeliveryInput);
    if (bytes < sizeof(InputStorage)) { bytes = sizeof(InputStorage); }
    InputStorage *storage = source_arena_allocate(arena, bytes);
    if (!storage) { goto no_memory; }
    size_t index = 0;
    for (const SemanticSymbol *field = root->first; field; field = field->next) {
        if (field->role != SYMBOL_PARAMETER) { continue; }
        if (field->owner != root || field->role_ordinal != index) {
            *error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH, .primary = {.span = field->site},
                .message = "Checked input declaration identity or order is inconsistent"};
            return false;
        }
        bool text = !field->reference && !field->list && field->type->module->syntax.sections.kind == MODULE_NATIVE &&
            field->type->abi.bindings[ABI_LITERAL].present;
        storage->entries[index] = (DeliveryInput){field, index, text}; index++;
    }
    storage->table = (DeliveryInputs){selection, count ? storage->entries : NULL, count};
    *output = &storage->table;
    return true;
no_memory:
    *error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY, .message = "Cannot retain the complete declaration-ordered delivery input table"};
    return false;
}
