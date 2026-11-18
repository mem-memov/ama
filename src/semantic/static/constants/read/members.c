/** @file
 * @brief Bind selected own constant declarations after concrete namespace lookup.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) selects only declared own shared values.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) preserves dependent member Type terms.
 */
#include "../internal.h"

/** Normalized saved-read requests retain receiver syntax but cannot supply
 * namespace inference. Member Type constraints are scoped to the checked actual
 * namespace, independently of caller receiver names and inferred runtime storage.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) orders lookup before typed copying.
 */
bool static_constant_members(StaticConstantPlan *plan, StaticConstantSelection *selection) {
    StaticConstantMember **tail = &selection->first;
    for (const BindingRequest *request = selection->requests->first; request; request = request->next) {
        if (request->direction != BINDING_RUNTIME_OUTPUT || request->slot == NULL || request->slot->runtime_declaration == NULL) {
            return static_constant_fail(plan, STATIC_CONSTANT_INVALID, request->span, "Constant selection has no actual own shared declaration");
        }
        const StaticConstantDeclaration *declaration;
        if (!static_constant_declaration(plan, selection->context, request->slot->runtime_declaration, &declaration)) { return false; }
        StaticConstantMember *member = static_constant_allocate(plan, sizeof(*member));
        if (member == NULL) { return false; }
        *member = (StaticConstantMember){.request = request, .declaration = declaration};
        *tail = member;
        tail = &member->next;
        selection->count++;
    }
    return true;
}
