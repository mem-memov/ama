/** @file
 * @brief Resolve actual variable uses against fixed lexical locals and module declarations.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) forbids future inferred repair and inferred self-sourcing.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) excludes locals from creation-time access.
 */
#include <stdint.h>
#include "../../internal.h"

/** The caller supplies original syntax/span metadata. Resolve scope before
 * recording module encounters; a currently inferred field cannot supply itself.
 * A source initializer's constant-only restriction is known without Type solving.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes source bindings from the scanned prefix.
 */
bool static_runtime_use(StaticConnectionPlan *plan, StaticRuntimeModule *module,
    StaticRuntimeUse value, const StaticRuntimeField **output) {
    SourceSpan name = value.syntax ? value.syntax->name : value.site;
    const StaticRuntimeField *field = static_runtime_local(module, name, value.site);
    if (field) {
        if (value.initializing) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, name, "Creation-time initializers cannot read foreach-local storage");
        }
    } else {
        if (!static_runtime_encounter(plan, module, name, NULL)) { return false; }
        if (module->inferring.source && static_same_name(module->inferring, name)) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, name, "An inferred field cannot supply its own introducing assignment");
        }
        field = static_runtime_field(module, name);
        if (field == NULL) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, name, "Runtime name has no explicit or earlier inferred declaration");
        }
    }
    if (value.initializing && value.initializing->role == STATIC_FIELD_CONSTANT && field->role != STATIC_FIELD_CONSTANT) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, name, "Shared initialization cannot read instance storage");
    }
    if (module->use_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, value.site, "Runtime use count exceeds representable storage");
    }
    StaticRuntimeUse *use = static_connection_allocate(plan, sizeof(*use));
    if (use == NULL) { return false; }
    value.next = NULL;
    value.field = field;
    *use = value;
    if (module->last_use) { module->last_use->next = use; }
    else { module->first_use = use; }
    module->last_use = use;
    module->use_count++;
    *output = field;
    return true;
}
