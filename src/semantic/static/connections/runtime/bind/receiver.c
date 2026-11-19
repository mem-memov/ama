/** @file
 * @brief Classify runtime receiver identities exactly from the written prefix before Type solving.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes fresh versus existing storage.
 * [NAME-04](../../../../../../spec/language/declarations.md#name-04) forbids inferred/explicit duplicates.
 */
#include <stdint.h>
#include "../../internal.h"

/** Lexical locals update their existing cells. Only unseen simple module names
 * can introduce fields; hoisted explicit declarations still produce duplicates.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) forbids classification repair after inference.
 */
static bool simple(StaticConnectionPlan *plan, StaticRuntimeModule *module, const StaticRuntimeEvent *event,
    const AstAccess *syntax, const StaticRuntimeField **output, bool *introduced) {
    const StaticRuntimeField *field = static_runtime_local(module, syntax->name, syntax->span);
    if (field == NULL) {
        bool earlier;
        if (!static_runtime_encounter(plan, module, syntax->name, &earlier)) { return false; }
        field = static_runtime_field(module, syntax->name);
        if (!earlier) {
            if (field) { return static_runtime_duplicate(plan, module, field->site, syntax->span); }
            StaticRuntimeField value = {.name = syntax->name, .site = syntax->span, .role = STATIC_FIELD_STATE,
                .statement = event->node, .receiver = syntax, .reference = syntax->reference,
                .list = syntax->whole_list, .inferred = true};
            if (!static_runtime_add_field(plan, module, value, &field)) { return false; }
            *introduced = true;
        } else if (field == NULL) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, syntax->span, "Earlier use prevents declaring this receiver");
        }
    }
    *output = field;
    return true;
}

/** Indexed receivers visit their existing collection/index names and never
 * declare either. Written suffixes and shared immutability are checked before
 * recording a transfer; actual source/result Types remain independent work.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) preserves exact receiver storage.
 */
bool static_runtime_receiver(StaticConnectionPlan *plan, StaticRuntimeModule *module,
    const StaticRuntimeEvent *event, const AstAccess *syntax, const StaticRuntimeReceiver **output) {
    if (syntax == NULL) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, event->site, "Runtime receiver has no original access");
    }
    const StaticRuntimeField *field;
    bool introduced = false;
    if (syntax->index) {
        if (!static_runtime_access(plan, module, event, syntax, false, &field)) { return false; }
    } else if (!simple(plan, module, event, syntax, &field, &introduced)) { return false; }
    if (field->role == STATIC_FIELD_CONSTANT) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, syntax->span, "Shared constant storage is immutable");
    }
    if (field->reference != syntax->reference || (syntax->index == NULL && field->list != syntax->whole_list)) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, syntax->span, "Runtime receiver storage suffix differs from its declaration");
    }
    if (module->receiver_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, syntax->span, "Runtime receiver count exceeds representable storage");
    }
    StaticRuntimeReceiver *receiver = static_connection_allocate(plan, sizeof(*receiver));
    if (receiver == NULL) { return false; }
    *receiver = (StaticRuntimeReceiver){.statement = event->node, .syntax = syntax, .field = field, .introduced = introduced};
    if (module->last_receiver) { module->last_receiver->next = receiver; }
    else { module->first_receiver = receiver; }
    module->last_receiver = receiver;
    module->receiver_count++;
    *output = receiver;
    return true;
}
