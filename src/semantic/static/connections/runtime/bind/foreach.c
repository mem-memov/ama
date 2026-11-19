/** @file
 * @brief Fix foreach source/binder identities at the header without Type-dependent shadowing.
 * [FLOW-05](../../../../../../spec/language/lists.md#flow-05) reuses visible mutable scalar references.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) scopes only newly introduced cells.
 */
#include <stdint.h>
#include "../../internal.h"

/** Resolve the source first, then reuse visible reference storage or create one
 * exact body-local binding. Element Type compatibility remains a later equation;
 * its unknown value cannot change this lexical choice or create shadowing.
 * [FLOW-05](../../../../../../spec/language/lists.md#flow-05) fixes header ordering and list/reference form.
 */
bool static_runtime_foreach(StaticConnectionPlan *plan, StaticRuntimeModule *module, const StaticRuntimeEvent *event) {
    const AstNode *node = event->node;
    StaticRuntimeUse use = {.statement = node, .site = node->as.traversal.collection};
    const StaticRuntimeField *collection;
    if (!static_runtime_use(plan, module, use, &collection)) { return false; }
    if (collection->role == STATIC_FIELD_CONSTANT || !collection->list || collection->reference != node->as.traversal.reference_elements) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, use.site, "Foreach requires matching mutable list storage");
    }
    SourceSpan name = node->as.traversal.binder;
    const StaticRuntimeField *binding = static_runtime_local(module, name, name);
    if (binding == NULL) { binding = static_runtime_field(module, name); }
    bool introduced = binding == NULL;
    if (binding) {
        if (binding->role == STATIC_FIELD_CONSTANT || !binding->reference || binding->list) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, name, "Foreach cannot shadow incompatible visible storage");
        }
        use.site = name;
        if (!static_runtime_use(plan, module, use, &binding)) { return false; }
    } else {
        StaticRuntimeField field = {.name = name, .site = name, .role = STATIC_FIELD_STATE,
            .statement = node, .scope = node, .reference = true};
        if (!static_runtime_add_field(plan, module, field, &binding)) { return false; }
    }
    if (module->foreach_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, name, "Foreach binding count exceeds representable storage");
    }
    StaticRuntimeForeach *traversal = static_connection_allocate(plan, sizeof(*traversal));
    if (traversal == NULL) { return false; }
    *traversal = (StaticRuntimeForeach){.syntax = node, .collection = collection, .binding = binding, .introduced = introduced};
    if (module->last_foreach) { module->last_foreach->next = traversal; }
    else { module->first_foreach = traversal; }
    module->last_foreach = traversal;
    module->foreach_count++;
    return true;
}
