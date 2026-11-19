/** @file
 * @brief Resolve typed field associations by retained original lexical identities rather than names.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) forbids Type-dependent rebinding.
 */
#include "../../internal.h"

/** Exact scope pointer membership precedes borrowed payload access.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) preserves owner boundaries.
 */
bool static_connection_has_scope(const StaticConnectionPlan *plan, const StaticTypedScope *candidate) {
    for (const StaticTypedScope *scope = plan->first_scope; scope; scope = scope->next) {
        if (scope == candidate) { return true; }
    }
    return false;
}

/** Corresponding RuntimeField pointers were fixed by the source-only name pass;
 * same spelling in another scope or module cannot select a typed association.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) preserves lexical declaration identity.
 */
bool static_connection_field(StaticConnectionPlan *plan, const StaticTypedScope *scope,
    const StaticRuntimeField *definition, const StaticTypedField **output) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (output == NULL || !static_connection_has_scope(plan, scope)) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Invalid typed scope or field receiver");
    }
    for (const StaticTypedField *field = scope->first_field; field; field = field->next) {
        if (field->definition == definition) { *output = field; return true; }
    }
    return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Typed field does not belong to this original runtime scope");
}
