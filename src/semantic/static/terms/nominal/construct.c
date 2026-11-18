/** @file
 * @brief Intern normalized nominal records using exact original formal mappings.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) defines nominal identity.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial allocation.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) leaves equation solving to its separate owner.
 */
#include <stdint.h>
#include "internal.h"

/** Validate complete array membership and uniqueness before dereferencing keys.
 * Categories/equations belong to application checking, not representation.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) separates constraints.
 */
static bool valid_captures(StaticTermStore *store, const StaticModule *origin,
    const StaticTermCapture *captures, size_t count) {
    if (count > origin->parameter_count || (count != 0 && captures == NULL)) { return false; }
    for (size_t index = 0; index < count; index++) {
        const StaticSymbol *formal = captures[index].formal;
        if (!static_term_has_symbol(store, formal) || formal->owner != origin ||
            formal->role != STATIC_PARAMETER || !static_term_has_value(store, captures[index].value)) { return false; }
        for (size_t previous = 0; previous < index; previous++) {
            if (captures[previous].formal == formal) { return false; }
        }
    }
    return true;
}

/** Child terms are already interned, so exact child pointer comparisons are
 * structural equality by induction on construction order, without recursion.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) retains every capture.
 */
static bool same_nominal(const StaticTerm *term, const StaticModule *origin,
    const StaticTermCapture *captures, size_t count) {
    if (term->kind != STATIC_TERM_NOMINAL || term->as.nominal.origin != origin ||
        term->as.nominal.capture_count != count) { return false; }
    for (const StaticTermCapture *item = term->as.nominal.first_capture; item; item = item->next) {
        if (static_nominal_supplied(captures, count, item->formal) != item->value) { return false; }
    }
    return true;
}

/** Source provenance is retained on first creation but never enters value keys.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) shares specialization identity.
 */
bool static_term_nominal(StaticTermStore *store, const StaticModule *origin,
    const StaticTermCapture *captures, size_t count, SourceSpan site, const StaticTerm **output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || !static_term_has_module(store, origin) ||
        !valid_captures(store, origin, captures, count)) {
        return static_term_invalid(store, site, "Invalid normalized nominal capture record");
    }
    for (StaticTerm *term = store->first; term != NULL; term = term->next) {
        if (same_nominal(term, origin, captures, count)) { *output = term; return true; }
    }
    if (store->term_count == SIZE_MAX) { return static_term_memory_failure(store); }
    StaticTerm *term = static_term_allocate(store, sizeof(*term));
    if (term == NULL) { return false; }
    term->owner = store;
    term->ordinal = store->term_count;
    term->kind = STATIC_TERM_NOMINAL;
    term->site = site;
    term->as.nominal.origin = origin;
    if (!static_nominal_prepare(store, term, captures, count)) { return false; }
    if (store->last != NULL) { store->last->next = term; }
    else { store->first = term; }
    store->last = term;
    store->term_count++;
    *output = term;
    return true;
}
