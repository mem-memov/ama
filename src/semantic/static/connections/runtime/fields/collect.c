/** @file
 * @brief Hoist exact explicit runtime declarations without encountering names or requiring closed Types.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) requires the full explicit inventory first.
 * [NAME-04](../../../../../../spec/language/declarations.md#name-04) prohibits duplicate runtime base names.
 */
#include "../../internal.h"

/** Section role is original parser/inventory metadata, never inferred from uses.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) keeps static names outside runtime storage.
 */
static bool role(StaticConnectionPlan *plan, const StaticRuntimeEvent *event, StaticRuntimeRole *output) {
    switch (event->context) {
        case STATIC_CONTEXT_CONSTANT: *output = STATIC_FIELD_CONSTANT; return true;
        case STATIC_CONTEXT_INPUT: *output = STATIC_FIELD_INPUT; return true;
        case STATIC_CONTEXT_RESULT: *output = STATIC_FIELD_RESULT; return true;
        case STATIC_CONTEXT_BODY: *output = STATIC_FIELD_STATE; return true;
        default: return static_connection_fail(plan, STATIC_CONNECTION_INVALID, event->site, "Runtime declaration has no written runtime section role");
    }
}

/** Sorted declaration events make duplicate provenance deterministic. Original
 * Type expressions remain symbolic source references; no body/default executes.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) hoists visibility, not source encounters.
 */
bool static_runtime_fields(StaticConnectionPlan *plan, StaticRuntimeModule *module) {
    for (const StaticRuntimeEvent *event = module->first_event; event; event = event->next) {
        if (event->kind != STATIC_RUNTIME_DECLARATION) { continue; }
        const AstNode *node = event->node;
        const AstAccess *name = node->as.declaration.name;
        const StaticRuntimeField *prior = static_runtime_field(module, name->name);
        if (prior) { return static_runtime_duplicate(plan, module, name->span, prior->site); }
        StaticRuntimeField field = {.name = name->name, .site = name->span, .statement = node,
            .type_expression = node->as.declaration.type, .reference = name->reference, .list = name->whole_list};
        if (!role(plan, event, &field.role)) { return false; }
        if (field.role == STATIC_FIELD_CONSTANT && field.reference) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, field.site, "Shared runtime declarations cannot store mutable references");
        }
        const StaticRuntimeField *published;
        if (!static_runtime_add_field(plan, module, field, &published)) { return false; }
    }
    return true;
}
