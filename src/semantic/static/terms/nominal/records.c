/** @file
 * @brief Partition original formals into ordered captures and local residual binders.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) retains unused captures.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) distinguishes residuals from holes.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns unpublished capture and signature records.
 */
#include "internal.h"

/** The input is an explicit array, not its possibly unset linked-list members.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) preserves exact keys.
 */
const StaticTerm *static_nominal_supplied(const StaticTermCapture *captures, size_t count,
    const StaticSymbol *formal) {
    for (size_t index = 0; index < count; index++) {
        if (captures[index].formal == formal) { return captures[index].value; }
    }
    return NULL;
}

/** Each original formal enters exactly one ordered partition; signatures borrow
 * original nested metadata. [TPL-08](../../../../../spec/language/templates.md#tpl-08).
 */
bool static_nominal_prepare(StaticTermStore *store, StaticTerm *term,
    const StaticTermCapture *captures, size_t count) {
    StaticNominalTerm *nominal = &term->as.nominal;
    StaticSignature *signature = NULL;
    if (nominal->origin->parameter_count != count) {
        signature = static_term_allocate(store, sizeof(*signature));
        if (signature == NULL) { return false; }
    }
    term->ground = true;
    for (const StaticSymbol *formal = nominal->origin->first; formal != NULL; formal = formal->next) {
        if (formal->role != STATIC_PARAMETER) { continue; }
        const StaticTerm *value = static_nominal_supplied(captures, count, formal);
        if (value != NULL) {
            StaticTermCapture *capture = static_term_allocate(store, sizeof(*capture));
            if (capture == NULL) { return false; }
            capture->formal = formal;
            capture->value = value;
            if (nominal->last_capture != NULL) { nominal->last_capture->next = capture; }
            else { nominal->first_capture = capture; }
            nominal->last_capture = capture;
            nominal->capture_count++;
            term->ground = term->ground && value->ground;
        } else {
            StaticTermResidual *residual = static_term_allocate(store, sizeof(*residual));
            StaticSignatureEntry *entry = static_term_allocate(store, sizeof(*entry));
            if (residual == NULL || entry == NULL) { return false; }
            residual->formal = formal;
            if (nominal->last_residual != NULL) { nominal->last_residual->next = residual; }
            else { nominal->first_residual = residual; }
            nominal->last_residual = residual;
            nominal->residual_count++;
            entry->name = formal->name;
            entry->category = formal->category;
            entry->signature = formal->signature;
            if (signature->last != NULL) { signature->last->next = entry; }
            else { signature->first = entry; }
            signature->last = entry;
            signature->count++;
        }
    }
    term->category = signature != NULL ? STATIC_CATEGORY_TEMPLATE : STATIC_CATEGORY_TYPE;
    term->signature = signature;
    return true;
}
