/** @file
 * @brief Preserve every resolved operation, binding, traversal and control edge across a candidate rename.
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08) keeps caller/callee metadata distinct;
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) requires a complete graph correspondence.
 */
#include "internal.h"
/** Bindings retain their independent direction/ordinal, exact interface descriptor and transfer mode. */
static bool binding(const RenameIdentity *check, const SemanticBinding *old, const SemanticBinding *fresh) {
    return old->position == fresh->position && old->reference == fresh->reference && old->list == fresh->list
        && old->transfer == fresh->transfer && old->intrinsic == fresh->intrinsic
        && old->syntax->kind == fresh->syntax->kind && old->syntax->omitted == fresh->syntax->omitted
        && rename_same_symbol(check,old->field,fresh->field) && rename_same_type(old->type,fresh->type)
        && rename_same_span(check,old->syntax->span,fresh->syntax->span)
        && rename_same_span(check,old->syntax->marker,fresh->syntax->marker)
        && rename_same_source(check,old->source,fresh->source) && rename_same_access(check,old->receiver,fresh->receiver);
}
/** A selected condition retains storage/result shape and source identity rather than a coincidentally equal declared type. */
static bool selection(const RenameIdentity *check, SemanticSelection old, SemanticSelection fresh) {
    return old.reference == fresh.reference && old.list == fresh.list && old.intrinsic == fresh.intrinsic && old.position == fresh.position
        && rename_same_type(old.type,fresh.type) && rename_same_symbol(check,old.field,fresh.field)
        && rename_same_access(check,old.access,fresh.access);
}
/** New-local versus reused-reference traversal behavior is observable even when a captured candidate compiles successfully. */
static bool traversal(const RenameIdentity *check, const SemanticForeachBinding *old, const SemanticForeachBinding *fresh) {
    return old == NULL ? fresh == NULL : fresh != NULL && old->introduced == fresh->introduced
        && rename_same_symbol(check,old->collection,fresh->collection) && rename_same_symbol(check,old->binding,fresh->binding)
        && rename_same_span(check,old->syntax->span,fresh->syntax->span)
        && old->syntax->as.traversal.reference_elements == fresh->syntax->as.traversal.reference_elements;
}
/** Ordered action links must still identify their exact checked operation/binding, not another same-typed target. */
static bool actions(const RenameIdentity *check, const SemanticOutputAction *old, const SemanticOutputAction *fresh) {
    while (old != NULL && fresh != NULL) {
        if (old->syntax->kind != fresh->syntax->kind || !rename_same_link(check,old->operation,fresh->operation)
            || (old->binding == NULL ? fresh->binding != NULL
                : fresh->binding == NULL || !binding(check,old->binding,fresh->binding))) { return false; }
        old = old->next;
        fresh = fresh->next;
    }
    return old == NULL && fresh == NULL;
}
/** Flat statement inventory makes cyclic alias/control checks finite and nonrecursive. */
bool rename_same_statement(const RenameIdentity *check, const SemanticStatement *old, const SemanticStatement *fresh) {
    if (!rename_same_link(check,old,fresh) || old->empty_body != fresh->empty_body || old->negative_branch != fresh->negative_branch
        || old->operation_kind != fresh->operation_kind || old->list_method != fresh->list_method || old->transfer != fresh->transfer
        || old->intrinsic_target != fresh->intrinsic_target || old->target_reference != fresh->target_reference || old->intrinsic_result != fresh->intrinsic_result
        || !rename_same_type(old->namespace_type,fresh->namespace_type)
        || !rename_same_type(old->list_interface.element,fresh->list_interface.element)
        || !rename_same_type(old->list_interface.integer,fresh->list_interface.integer)
        || old->list_interface.reference != fresh->list_interface.reference
        || !rename_same_link(check,old->enclosing_operation,fresh->enclosing_operation)
        || !rename_same_link(check,old->enclosing_loop,fresh->enclosing_loop)
        || !rename_same_link(check,old->loop_root,fresh->loop_root)
        || !rename_same_link(check,old->condition_owner,fresh->condition_owner)
        || !rename_same_link(check,old->list_service,fresh->list_service)
        || !selection(check,old->condition,fresh->condition) || !traversal(check,old->traversal,fresh->traversal)
        || !rename_same_source(check,old->source,fresh->source) || !rename_same_access(check,old->access,fresh->access)
        || !actions(check,old->actions,fresh->actions)) { return false; }
    const SemanticBinding *a = old->bindings, *b = fresh->bindings;
    while (a != NULL && b != NULL) {
        if (!binding(check,a,b)) { return false; }
        a = a->next;
        b = b->next;
    }
    return a == NULL && b == NULL;
}
