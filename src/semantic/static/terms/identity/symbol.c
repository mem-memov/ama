/** @file
 * @brief Resolve local static symbols in their explicit generic or applied owner.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) forbids spelling-based capture.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) retrieves captures by original formal identity.
 */
#include "../internal.h"

/** Applied formals read original captures; derived symbols remain scoped variables.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) preserves original formals.
 */
bool static_term_symbol(StaticTermStore *store, const StaticTermEnvironment *environment,
    const StaticSymbol *symbol, const StaticTerm **output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || !static_term_has_environment(store, environment) ||
        !static_term_has_symbol(store, symbol) || symbol->owner != environment->definition) {
        return static_term_invalid(store, (SourceSpan){0}, "Invalid scoped static symbol identity");
    }
    if (symbol->role == STATIC_PARAMETER && environment->owner_type != NULL) {
        for (const StaticTermCapture *capture = environment->owner_type->as.nominal.first_capture;
            capture != NULL; capture = capture->next) {
            if (capture->formal == symbol) { *output = capture->value; return true; }
        }
        return static_term_invalid(store, symbol->site, "Applied owner lacks an original formal capture");
    }
    if (symbol->role == STATIC_PARAMETER && environment->producer_applied) {
        for (const StaticTermCapture *input = environment->producer_inputs; input; input = input->next) {
            if (input->formal == symbol) { *output = input->value; return true; }
        }
        return static_term_invalid(store, symbol->site, "Static producer lacks an original input binding");
    }
    StaticVariableIdentity identity = {.environment = environment, .symbol = symbol,
        .role = symbol->role == STATIC_PARAMETER ? STATIC_VARIABLE_FORMAL : STATIC_VARIABLE_DERIVED};
    return static_term_variable(store, identity, symbol->category, symbol->signature, symbol->site, output);
}
