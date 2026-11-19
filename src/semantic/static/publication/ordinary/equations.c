/** @file
 * @brief Generate only original ordinary annotation facts without expanding an ordinary body.
 * [TPL-04](../../../../../spec/language/templates.md#tpl-04) gives omitted field arguments exact source identities.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits finite structural equations.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves separate actual expansion checks.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires independently concrete exports.
 */
#include "internal.h"
#include "../progress.h"

/** Plain ordinary syntax has no nested static binding/definition evaluation.
 * Shared operand/application kernels retain only its exact finite equations.
 * An export instead closes its actual independent static root, never an ordinary
 * caller root. Such metadata work cannot certify an actual ordinary use.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) leaves ancestry/body validation outstanding.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids reverse supplier inference.
 */
bool static_ordinary_equations(StaticPublicationPlan *plan, StaticOrdinaryType *type, bool *output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (!output || !static_ordinary_has(plan, type)) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Invalid ordinary annotation or progress receiver");
    }
    StaticPublicationProgress before = static_publication_progress(plan);
    if (type->producer) {
        if (!static_ordinary_export(plan, type)) { return false; }
    } else if (!static_result_operands(plan->results, type->annotation, &type->prepared) ||
        !static_result_application(plan->results, type->annotation, type->prepared, &type->application)) {
        (void)static_publication_dependencies(plan); return false;
    }
    *output = !static_publication_same_progress(before, static_publication_progress(plan));
    return true;
}
