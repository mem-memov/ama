/** @file
 * @brief Submit finite field annotation facts without certifying an ordinary body.
 * [TPL-04](../../../../../../spec/language/templates.md#tpl-04) permits per-field inference variables.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) separates metadata from guarded expansion.
 */
#include "../../internal.h"

/** Original typed fields retain the already registered ordinary annotation.
 * Inferred receiver/local holes have no fabricated Type expression to evaluate.
 * Independent static exports are resolved by their own publication service.
 * [TPL-04](../../../../../../spec/language/templates.md#tpl-04) preserves exact original field identities.
 */
bool static_connection_annotations(StaticConnectionPlan *plan, const StaticTypedScope *scope, bool *output) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!output || !static_connection_has_scope(plan, scope)) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Invalid typed annotation scope or progress receiver");
    }
    bool changed = false;
    for (const StaticTypedField *field = scope->first_field; field; field = field->next) {
        if (!field->type_source) { continue; }
        bool local;
        if (!static_ordinary_equations(plan->constants->publications, field->type_source, &local)) {
            (void)static_connection_dependencies(plan); return false;
        }
        changed |= local;
    }
    *output = changed;
    return true;
}
