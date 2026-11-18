/** @file
 * @brief Register original qualified sources through collected loaded dependency identities.
 * [CONST-02](../../../../../spec/language/declarations.md#const-02) preserves written namespace provenance.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial registration.
 */
#include <stdint.h>
#include "../internal.h"

/** Validate source pointer membership before dereferencing arbitrary caller input.
 * No textual namespace search or synthetic expression participates in registration.
 * [CONST-02](../../../../../spec/language/declarations.md#const-02) distinguishes qualified sources from placeholders.
 */
static const StaticConstantOccurrence *occurrence(const StaticResultContext *context, const AstSource *source) {
    const StaticModule *module = context->creation->owner->environment->definition;
    for (const StaticConstantOccurrence *site = module->first_constant_source; site; site = site->next) {
        if (site->source == source) { return site; }
    }
    return NULL;
}

/** Registration retains an unresolved constructor as an obligation; selection
 * must reject it unless this written namespace independently denotes Type.
 * Exact caller-context/source identity makes unchanged registration idempotent.
 * [CONST-05](../../../../../spec/language/declarations.md#const-05) forbids receiver-driven completion.
 */
bool static_constant_source(StaticConstantPlan *plan, const StaticResultContext *context,
    const AstSource *syntax, const StaticConstantSource **output) {
    if (!static_constant_dependencies(plan)) { return false; }
    StaticResultPlan *results = plan->publications->results;
    if (output == NULL || !static_result_has_context(results, context)) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, (SourceSpan){0}, "Invalid qualified constant context or receiver");
    }
    const StaticConstantOccurrence *site = occurrence(context, syntax);
    if (site == NULL) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, (SourceSpan){0}, "Qualified source is not an original inventoried occurrence");
    }
    for (const StaticConstantSource *source = plan->first_source; source; source = source->next) {
        if (source->context == context && source->occurrence == site) { *output = source; return true; }
    }
    if (plan->source_count == SIZE_MAX) {
        return static_constant_fail(plan, STATIC_CONSTANT_OUT_OF_MEMORY, syntax->span, "Qualified source count exceeds representable storage");
    }
    StaticTermStore *terms = results->guards->applications->terms;
    const StaticModule *module = static_module_find(terms->inventory, site->origin);
    const StaticTerm *type;
    if (module == NULL) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, syntax->module_path, "Qualified source has no collected loaded namespace");
    }
    if (!static_term_nominal(terms, module, NULL, 0, syntax->module_path, &type)) {
        (void)static_constant_dependencies(plan); return false;
    }
    StaticConstantSource *source = static_constant_allocate(plan, sizeof(*source));
    if (source == NULL) { return false; }
    *source = (StaticConstantSource){.context = context, .occurrence = site, .namespace_type = type, .next = plan->first_source};
    plan->first_source = source;
    plan->source_count++;
    *output = source;
    return true;
}
