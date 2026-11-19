/** @file
 * @brief Merge immutable original capture maps with contextual static input inference.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves residuals and all prior bindings.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) distinguishes producer inference.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** Capture presence is a binding even when its value is a symbolic variable.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) distinguishes holes and residuals.
 */
static bool contains(const StaticTermCapture *captures, size_t count, const StaticSymbol *formal) {
    for (size_t index = 0; index < count; index++) {
        if (captures[index].formal == formal) { return true; }
    }
    return false;
}

/** Only ordinary field policy creates field holes. Static producer inputs have
 * their own graph solver. [TPL-04](../../../../../spec/language/templates.md#tpl-04)
 * fixes identity to the declaring owner and field, regardless of written section.
 */
static bool infer(StaticApplicationPlan *plan, StaticApplicationRecord *record,
    const StaticSymbol *formal, const StaticTerm **value) {
    StaticApplicationContext context = record->context;
    if (!static_term_inference(plan->terms, context.environment, formal,
        context.declaration, NULL, value)) {
        (void)static_application_dependencies(plan);
        return false;
    }
    StaticApplicationInput input = {.formal = formal, .value = *value, .site = context.site, .generated = true};
    return static_application_input(plan, record, input);
}

/** The candidate's original formals partition into retained/supplied captures
 * and local residuals. Never dereference argument variables into a different
 * lexical identity or execute a runtime default to fill a slot.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) fixes structural application.
 */
bool static_application_merge(StaticApplicationPlan *plan, StaticApplicationRecord *record) {
    const StaticTerm *constructor = record->constructor;
    const StaticModule *origin = constructor->as.nominal.origin;
    size_t maximum = origin->parameter_count;
    if (maximum > SIZE_MAX / sizeof(StaticTermCapture)) {
        return static_application_fail(plan, STATIC_APPLY_OUT_OF_MEMORY, record->context.site,
            "Static capture map exceeds representable storage", NULL);
    }
    StaticTermCapture *captures = maximum != 0 ? static_application_allocate(plan, maximum * sizeof(*captures)) : NULL;
    if (maximum != 0 && captures == NULL) { return false; }
    size_t count = 0;
    for (const StaticTermCapture *item = constructor->as.nominal.first_capture; item; item = item->next) {
        captures[count++] = (StaticTermCapture){.formal = item->formal, .value = item->value};
    }
    for (const StaticApplicationInput *input = record->first; input; input = input->next) {
        captures[count++] = (StaticTermCapture){.formal = input->formal, .value = input->value};
        record->applied = true;
    }
    for (size_t ordinal = 0; ordinal < maximum; ordinal++) {
        const StaticSymbol *formal = static_symbol_at(origin, STATIC_PARAMETER, ordinal);
        if (contains(captures, count, formal)) { continue; }
        if (record->context.policy == STATIC_OMIT_PARTIAL) { continue; }
        if (record->context.policy == STATIC_OMIT_NAMESPACE) {
            return static_application_fail(plan, STATIC_APPLY_CONFLICT, record->context.site,
                "This type expression requires explicit coverage of every residual formal", formal);
        }
        const StaticTerm *value;
        if (!infer(plan, record, formal, &value)) { return false; }
        captures[count++] = (StaticTermCapture){.formal = formal, .value = value};
        record->applied = true;
    }
    if (!static_term_nominal(plan->terms, origin, captures, count, record->context.site, &record->value)) {
        (void)static_application_dependencies(plan);
        return false;
    }
    return true;
}
