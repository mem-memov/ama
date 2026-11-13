/** @file
 * @brief Validate lifecycle graph membership, typed fields, hooks and source maps.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) separates typed
 * recipes from emitted text; [LIFE-03](../../../../spec/implementation/native-interface.md#life-03)
 * requires explicit cleanup edges. Allocation inventories are distinct from execution links.
 */
#include "internal.h"
/** Membership scans never follow executable cycles or dereference a foreign edge. */
static bool member(const PlannedType *type, const LifeNode *candidate) {
    for (const LifeNode *node = type->lifecycles; node != NULL; node = node->allocated_next) {
        if (node == candidate) { return true; }
    }
    return false;
}
/** ABI pointers must name a registered binding in the exact semantic owner. */
static bool hook(const PlannedType *type, const LifeNode *node) {
    if (type->type->module->syntax.sections.kind != MODULE_NATIVE) { return false; }
    if (node->operation == LIFE_CONSTRUCT_NATIVE) {
        return (node->binding == &type->type->abi.bindings[ABI_DEFAULT] && node->binding->present)
            || (node->binding == &type->type->abi.bindings[ABI_LITERAL] && node->binding->present);
    }
    size_t index = node->operation == LIFE_DESTROY_NATIVE ? ABI_DESTROY : ABI_CLONE;
    return node->binding == &type->type->abi.bindings[index] && node->binding->present;
}

bool plan_ir_lifecycle(ProgramPlan *plan, const PlannedType *type) {
    if (type->requires_pool && (type->create.first == NULL || type->destroy.first == NULL
            || (type->type->cloneable && (type->clone.first == NULL || type->continuation.first == NULL))
            || (type->type->abi.bindings[ABI_LITERAL].present && type->literal.first == NULL))) {
        return plan_fail(plan, (SourceSpan){type->type->module->source->text, 0, 0},
            "Typed IR is missing a required lifecycle procedure");
    }
    const LifeProcedure *procedures[] = {&type->create, &type->literal, &type->destroy,
        &type->clone, &type->continuation};
    for (size_t index = 0; index < sizeof(procedures) / sizeof(*procedures); index++) {
        const LifeProcedure *procedure = procedures[index];
        if ((procedure->first != NULL && !member(type, procedure->first))
            || (procedure->last != NULL && !member(type, procedure->last))
            || (procedure->failure != NULL && !member(type, procedure->failure))) {
            return plan_fail(plan, (SourceSpan){type->type->module->source->text, 0, 0},
                "Typed IR lifecycle endpoint has no owning instruction");
        }
    }
    for (const LifeNode *node = type->lifecycles; node != NULL; node = node->allocated_next) {
        bool field = node->operation == LIFE_CONSTRUCT_FIELD || node->operation == LIFE_DESTROY_FIELD
            || node->operation == LIFE_CLONE_OWNED || node->operation == LIFE_COPY_REFERENCE;
        bool native = node->operation == LIFE_CONSTRUCT_NATIVE || node->operation == LIFE_DESTROY_NATIVE
            || node->operation == LIFE_CLONE_NATIVE;
        if (node->type != type || node->operation < LIFE_CONSTRUCT_FIELD || node->operation > LIFE_FAILURE
            || !source_span_valid(node->site) || node->site.source != type->type->module->source->text
            || (node->next != NULL && !member(type, node->next))
            || (node->failure != NULL && !member(type, node->failure))
            || (field ? !plan_ir_field(type, node->field) : node->field != NULL)
            || (native ? !hook(type, node) : node->binding != NULL)) {
            return plan_fail(plan, node->site, "Typed IR lifecycle instruction has an unresolved operand or foreign edge");
        }
    }
    return true;
}
