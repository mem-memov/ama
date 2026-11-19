/** @file
 * @brief Validate exact applied names against the independently known nominal result.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits recovery
 * only when each supplied original formal is fixed in that result.
 */
#include "internal.h"
#include "../../internal.h"

const BindingRequest *static_recovery_supplied(const StaticTermApplication *source, const StaticSymbol *formal) {
    for (const BindingRequest *request = source->source->first; request != NULL; request = request->next) {
        if (request->direction == BINDING_STATIC_INPUT && static_same_name(request->label, formal->name)) {
            return request;
        }
    }
    return NULL;
}

/** Original formal lookup uses only the actual result's known origin; matching
 * a spelling elsewhere cannot establish a constructor.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) forbids origin search.
 */
static const StaticSymbol *formal(const StaticModule *origin, SourceSpan label) {
    const StaticSymbol *symbol = static_symbol_find(origin, label);
    return symbol != NULL && symbol->role == STATIC_PARAMETER ? symbol : NULL;
}

bool static_recovery_names(StaticApplicationPlan *plan, const StaticTermApplication *source, const StaticTerm *actual) {
    for (const BindingRequest *request = source->source->first; request != NULL; request = request->next) {
        if (request->direction != BINDING_STATIC_INPUT) { continue; }
        const StaticSymbol *slot = formal(actual->as.nominal.origin, request->label);
        if (slot == NULL) {
            return static_application_fail(plan, STATIC_APPLY_CONFLICT, request->label,
                "An applied name is absent from the known result's original constructor", NULL);
        }
        bool fixed = false;
        for (const StaticTermCapture *capture = actual->as.nominal.first_capture; capture; capture = capture->next) {
            if (capture->formal == slot) { fixed = true; break; }
        }
        if (!fixed) {
            return static_application_fail(plan, STATIC_APPLY_CONFLICT, request->label,
                "An applied input must be fixed in the known application result", slot);
        }
    }
    return true;
}
