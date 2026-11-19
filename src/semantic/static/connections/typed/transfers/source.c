/** @file
 * @brief Derive local source Type and storage from immutable lexical use records.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) specifies the exact source storage table.
 */
#include "../../internal.h"

/** Match original statement/access pointers, not spelling or the final module
 * namespace. A foreach local and a later same-spelled field remain distinct.
 * The lexical pass has already checked suffixes and source ownership.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) preserves scoped cells.
 */
static const StaticRuntimeField *source_field(const StaticTypedScope *scope,
    const StaticRuntimeEvent *event, const AstAccess *access) {
    for (const StaticRuntimeUse *use = scope->runtime->first_use; use; use = use->next) {
        if (use->statement == event->node && use->syntax == access) { return use->field; }
    }
    return NULL;
}

/** Indexed lists yield scalar element Types; continuation copies yield owned
 * scalars even when their operand is a reference. Self-reference uses the full
 * nominal owner term. Qualified constants and literals require other services,
 * so they cannot silently contribute an invented Type anchor.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes these source forms.
 */
bool static_typed_operand(StaticConnectionPlan *plan, const StaticTypedScope *scope,
    const StaticRuntimeEvent *event, const AstSource *source, StaticTypedOperand *output,
    StaticResultOutcome *outcome) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!output || !outcome || !source || !event || !static_connection_has_scope(plan, scope)) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Invalid typed source or operand receiver");
    }
    if (source->kind == AST_LITERAL_SOURCE || source->kind == AST_CONSTANT_SOURCE) {
        *outcome = STATIC_RESULT_PENDING;
        return true;
    }
    StaticTypedOperand operand = {0};
    if (source->kind == AST_SELF_REFERENCE_SOURCE) {
        operand.type = scope->owner;
        operand.reference = true;
    } else {
        const StaticRuntimeField *definition = source_field(scope, event, source->access);
        if (!definition) {
            return static_connection_fail(plan, STATIC_CONNECTION_INVALID, source->span, "Typed source has no original lexical use");
        }
        const StaticTypedField *field;
        if (!static_connection_field(plan, scope, definition, &field)) { return false; }
        operand = (StaticTypedOperand){.type = field->type, .field = definition,
            .reference = definition->reference, .list = source->access->whole_list};
        if (source->kind == AST_ACQUIRE_SOURCE) { operand.reference = true; }
        if (source->kind == AST_CONTINUATION_SOURCE) {
            if (operand.list) {
                return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, source->span, "Continuation source must be scalar");
            }
            operand.reference = false;
        }
    }
    *output = operand;
    *outcome = STATIC_RESULT_READY;
    return true;
}
