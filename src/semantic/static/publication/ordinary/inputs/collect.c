/** @file
 * @brief Retain each original declaration input operator without C recursion.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) requires complete
 * structural operands; [TPL-02](../../../../../../spec/language/templates.md#tpl-02)
 * keeps every child in the enclosing definition's exact static environment.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) owns
 * all partial source/cache/frame allocations through refusal and teardown.
 */
#include <assert.h>
#include "internal.h"

/** Finite source cursor; parents and cursors borrow only arena/AST-owned data.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) permits input
 * expression trees, never output bindings in an ordinary declaration.
 */
typedef struct InputFrame {
    struct InputFrame *parent;
    const AstStaticBinding *binding;
} InputFrame;

/** Allocate a cursor only for a nonempty input region. Every pushed frame
 * follows a consumed owner edge; [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03)
 * retains popped frames for the same arena teardown as the child records.
 */
static InputFrame *push(StaticPublicationPlan *plan, InputFrame *parent,
    const AstStaticBinding *binding) {
    if (!binding) { return parent; }
    InputFrame *frame = static_publication_allocate(plan, sizeof(*frame));
    if (frame) { *frame = (InputFrame){parent, binding}; }
    return frame;
}

/** Consume one binding or pop a cursor on every transition. Child sources are
 * registered without entering them, as required by
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11). Preorder keeps
 * original sibling/parent ordering; no foreign label becomes a local operand.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) preserves the
 * root's enclosing environment for every original child.
 */
bool static_ordinary_collect_inputs(StaticPublicationPlan *plan, StaticOrdinaryType *type) {
    if (type->inputs_collected) { return true; }
    InputFrame *pending = push(plan, NULL, type->expression->first);
    if (type->expression->first && !pending) { return false; }
    StaticOrdinaryOperand **tail = &type->inputs;
    while (pending) {
        const AstStaticBinding *binding = pending->binding;
        if (!binding) { pending = pending->parent; continue; }
        assert(binding->kind == AST_STATIC_INPUT && binding->input);
        pending->binding = binding->next;
        const StaticResultSource *source;
        if (!static_result_source(plan->results, type->environment, binding->input, &source)) {
            (void)static_publication_dependencies(plan); return false;
        }
        StaticOrdinaryOperand *operand = static_publication_allocate(plan, sizeof(*operand));
        if (!operand) { return false; }
        operand->source = source;
        *tail = operand;
        tail = &operand->next;
        InputFrame *child = push(plan, pending, binding->input->first);
        if (binding->input->first && !child) { return false; }
        pending = child;
    }
    type->inputs_collected = true;
    return true;
}
