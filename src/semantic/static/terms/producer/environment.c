/** @file
 * @brief Intern compiler-only static producer scopes by their complete input maps.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) gives graph
 * producers an identity without introducing a third static value category.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains unused inputs.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial maps.
 */
#include <stdint.h>
#include "../internal.h"

/** Look up an original formal in an array, ignoring caller list links.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) uses named inputs.
 */
static const StaticTerm *supplied(const StaticTermCapture *inputs, size_t count,
    const StaticSymbol *formal) {
    for (size_t index = 0; index < count; index++) {
        if (inputs[index].formal == formal) { return inputs[index].value; }
    }
    return NULL;
}

/** Membership precedes dereference; cardinality plus uniqueness proves coverage.
 * Categories remain solver obligations, not assumptions of the identity layer.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains full keys.
 */
bool static_term_producer_inputs_valid(StaticTermStore *store, const StaticModule *definition,
    const StaticTermCapture *inputs, size_t count) {
    if (!static_term_has_module(store, definition) ||
        definition->definition->syntax.sections.kind != MODULE_STATIC ||
        count != definition->parameter_count || (count && inputs == NULL)) { return false; }
    for (size_t index = 0; index < count; index++) {
        const StaticSymbol *formal = inputs[index].formal;
        if (!static_term_has_symbol(store, formal) || formal->owner != definition ||
            formal->role != STATIC_PARAMETER || !static_term_has_value(store, inputs[index].value)) { return false; }
        for (size_t previous = 0; previous < index; previous++) {
            if (inputs[previous].formal == formal) { return false; }
        }
    }
    return true;
}

/** Exact interned child identities determine raw map equality; unknown variables
 * remain variables. Equality here is neither normalized-key equality nor permission
 * to expand a recursive producer. [TPL-11](../../../../../spec/language/templates.md#tpl-11).
 */
static bool same(const StaticTermEnvironment *environment, const StaticModule *definition,
    const StaticTermCapture *inputs, size_t count) {
    if (environment->definition != definition || !environment->producer_applied) { return false; }
    for (const StaticTermCapture *input = environment->producer_inputs; input; input = input->next) {
        if (supplied(inputs, count, input->formal) != input->value) { return false; }
    }
    return true;
}

/** Copy every binding before publishing; an allocation refusal cannot expose an
 * incomplete environment. Source-order copying makes caller permutation irrelevant.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves input identity.
 */
bool static_term_producer_environment(StaticTermStore *store, const StaticModule *definition,
    const StaticTermCapture *inputs, size_t count, const StaticTermEnvironment **output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || !static_term_producer_inputs_valid(store, definition, inputs, count)) {
        return static_term_invalid(store, (SourceSpan){0}, "Invalid static producer input map");
    }
    for (StaticTermEnvironment *item = store->first_environment; item; item = item->next) {
        if (same(item, definition, inputs, count)) { *output = item; return true; }
    }
    if (store->environment_count == SIZE_MAX) { return static_term_memory_failure(store); }
    StaticTermEnvironment *environment = static_term_allocate(store, sizeof(*environment));
    if (environment == NULL) { return false; }
    environment->definition = definition;
    environment->producer_applied = true;
    StaticTermCapture *last = NULL;
    for (size_t ordinal = 0; ordinal < count; ordinal++) {
        const StaticSymbol *formal = static_symbol_at(definition, STATIC_PARAMETER, ordinal);
        StaticTermCapture *input = static_term_allocate(store, sizeof(*input));
        if (input == NULL) { return false; }
        input->formal = formal;
        input->value = supplied(inputs, count, formal);
        if (last) { last->next = input; }
        else { environment->producer_inputs = input; }
        last = input;
    }
    if (store->last_environment) { store->last_environment->next = environment; }
    else { store->first_environment = environment; }
    store->last_environment = environment;
    store->environment_count++;
    *output = environment;
    return true;
}
