/** @file
 * @brief Check complete replacement groups, explicit commit points and capture-before-cleanup failure edges.
 * [REF-03](../../../../../spec/language/values-and-references.md#ref-03) preserves
 * old storage before commit; [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * does not undo earlier successful replacements after a later failure.
 */
#include "internal.h"
/** Test an instruction address against allocation ownership before dereferencing an executable edge. */
static bool node_member(const PlannedType *type, const ControlNode *candidate) {
    for (const ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
        if (node == candidate) { return true; }
    }
    return false;
}
/** Recipe ownership is independent of first/next execution links. */
static bool recipe_member(const PlannedType *type, const ReplacementPlan *candidate) {
    for (const ReplacementPlan *recipe = type->replacements; recipe != NULL; recipe = recipe->allocated_next) {
        if (recipe == candidate) { return true; }
    }
    return false;
}
/** No branch, return continuation or entry may bypass a group's selection/preflight prefix. */
static bool enters_inside(const ReplacementPlan *recipe, const ControlNode *target) {
    return target != NULL && target->replacement == recipe && target != recipe->first;
}
/** Every physical phase has one recipe and every recipe has exactly its complete ordered phase group. */
static bool group(const PlannedType *type, const ReplacementPlan *recipe) {
    ControlOperation first = recipe->context == REPLACE_ASSIGNMENT ? CONTROL_SELECT_RECEIVER
        : recipe->context == REPLACE_PARAMETER ? CONTROL_INSTALL_PARAMETER
        : recipe->context == REPLACE_RESULT ? CONTROL_INITIALIZE_RESULT : CONTROL_TRANSFER_OUTPUT;
    const ControlOperation phases[] = {first, CONTROL_SELECT_SOURCE, CONTROL_PREFLIGHT_REPLACEMENT,
        CONTROL_PREPARE_REPLACEMENT, CONTROL_COMMIT_REPLACEMENT, CONTROL_RELEASE_VALUES};
    const ControlNode *node = recipe->first;
    size_t count = 0;
    for (size_t index = 0; index < sizeof(phases) / sizeof(*phases); index++) {
        if (phases[index] == CONTROL_SELECT_SOURCE && recipe->context != REPLACE_ASSIGNMENT) { continue; }
        if (!node_member(type, node) || node->operation != phases[index] || node->replacement != recipe
            || node->statement != recipe->statement || node->binding != recipe->binding
            || node->failure != type->failure || node->alternative != NULL || node->resume != NULL) { return false; }
        if (phases[index] == CONTROL_COMMIT_REPLACEMENT && recipe->commit != node) { return false; }
        if (phases[index] == CONTROL_RELEASE_VALUES && recipe->last != node) { return false; }
        count++;
        node = node->next;
    }
    if (enters_inside(recipe, type->entry)) { return false; }
    size_t owned = 0;
    for (node = type->controls; node != NULL; node = node->allocated_next) {
        if (node->replacement == recipe) { owned++; }
        else if (enters_inside(recipe, node->next) || enters_inside(recipe, node->alternative)
            || enters_inside(recipe, node->resume) || enters_inside(recipe, node->failure)) { return false; }
    }
    return owned == count;
}
/** These instructions describe one replacement, never an unrelated unowned partial reset. */
static bool requires_recipe(ControlOperation operation) {
    return operation == CONTROL_SELECT_RECEIVER || operation == CONTROL_SELECT_SOURCE
        || operation == CONTROL_PREFLIGHT_REPLACEMENT || operation == CONTROL_PREPARE_REPLACEMENT
        || operation == CONTROL_COMMIT_REPLACEMENT || operation == CONTROL_RELEASE_VALUES
        || operation == CONTROL_INSTALL_PARAMETER || operation == CONTROL_INITIALIZE_RESULT
        || operation == CONTROL_TRANSFER_OUTPUT;
}
bool plan_ir_replacements(ProgramPlan *plan, const PlannedType *type) {
    for (const ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
        if ((requires_recipe(node->operation) && !recipe_member(type, node->replacement))
            || (!requires_recipe(node->operation) && node->replacement != NULL)) {
            return plan_fail(plan, node->site, "Replacement phase has no exact owned recipe");
        }
    }
    for (const ReplacementPlan *recipe = type->replacements; recipe != NULL; recipe = recipe->allocated_next) {
        if (!plan_ir_replacement_operands(type, recipe) || !group(type, recipe)) {
            return plan_fail(plan, (SourceSpan){type->type->module->source->text, 0, 0},
                "Replacement recipe has invalid operands, construction choice or phase order");
        }
    }
    return true;
}
