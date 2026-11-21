/** @file
 * @brief Generate original nested Type equations independently of body expansion.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) connects each
 * child result to its real operator. [TPL-04](../../../../../../spec/language/templates.md#tpl-04)
 * and [TPL-08](../../../../../../spec/language/templates.md#tpl-08) retain the
 * existing root FIELD versus nested PARTIAL omission policy.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) leaves all actual
 * paths, creation guards and executable validation with their current owners.
 */
#include "internal.h"

/** Preorder applies the same simultaneous structural equations as any other
 * schedule: input result variables already exist and later child equations
 * refine them. [TPL-10](../../../../../../spec/language/templates.md#tpl-10)
 * permits known nominal equalities without a search or inverse producer.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) keeps
 * partial preparations under their original owners when a child refuses.
 */
bool static_ordinary_input_equations(StaticPublicationPlan *plan, StaticOrdinaryType *type) {
    if (!static_ordinary_collect_inputs(plan, type)) { return false; }
    for (StaticOrdinaryOperand *operand = type->inputs; operand; operand = operand->next) {
        if (!static_result_operands(plan->results, operand->source, &operand->prepared) ||
            !static_result_application(plan->results, operand->source, operand->prepared, &operand->application)) {
            (void)static_publication_dependencies(plan); return false;
        }
    }
    return true;
}
