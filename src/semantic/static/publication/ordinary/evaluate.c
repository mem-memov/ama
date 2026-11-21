/** @file
 * @brief Advance actual ordinary Type expressions through their complete guarded source trees.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) separates ordinary fields and static supplier inputs.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) retains symbolic dependent Type evidence.
 */
#include "internal.h"

/** Every actual expression preserves its source evaluator's structural outcome,
 * separately from value and supplier closure. Failed output stays untouched.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) forbids reverse factory inference.
 */
bool static_ordinary_evaluate(StaticPublicationPlan *plan, StaticOrdinaryType *type,
    StaticResultOutcome *output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (!output || !static_ordinary_has(plan, type) || !type->context) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Invalid ordinary Type work or outcome receiver");
    }
    StaticResultOutcome outcome;
    if (!static_result_evaluate(plan->results, type->context, type->expression, &outcome)) {
        (void)static_publication_dependencies(plan); return false;
    }
    *output = outcome;
    return true;
}
