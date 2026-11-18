/** @file
 * @brief Register original shared initializer Type obligations without evaluating values.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) permits typed source evidence.
 * [CONST-02](../../../../../spec/language/declarations.md#const-02) restricts shared sources to constants and literals.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 */
#include <stdint.h>
#include "../internal.h"
#include "../../internal.h"

/** Validate both owned declaration identity and original initializer membership
 * before dereferencing caller-provided source pointers.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) preserves original obligations.
 */
static bool original(const StaticConstantPlan *plan, const StaticConstantDeclaration *candidate, const AstSource *source) {
    for (const StaticConstantDeclaration *declaration = plan->first_declaration; declaration; declaration = declaration->next) {
        if (declaration != candidate) { continue; }
        for (const AstInitializer *item = declaration->syntax->as.declaration.first; item; item = item->next) {
            if (item->source == source) { return true; }
        }
        return false;
    }
    return false;
}

/** A local source binds only its own shared namespace. Detect duplicate complete
 * base names rather than selecting one; ordinary initialization ordering is a
 * later value dependency check and cannot be bypassed by this Type evidence.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) excludes runtime arguments/fields.
 */
static bool local(StaticConstantPlan *plan, const StaticResultContext *context,
    const AstSource *source, const StaticConstantDeclaration **output) {
    const StaticModule *module = context->creation->owner->environment->definition;
    const AstNode *found = NULL;
    for (const AstNode *node = module->definition->syntax.constants.first; node; node = node->next) {
        if (node->kind != AST_DECLARATION || !static_same_name(node->as.declaration.name->name, source->access->name)) { continue; }
        if (found) {
            return static_constant_fail(plan, STATIC_CONSTANT_CONFLICT, source->span, "Initializer constant base name is ambiguous");
        }
        found = node;
    }
    if (found == NULL) {
        return static_constant_fail(plan, STATIC_CONSTANT_CONFLICT, source->span, "Shared initializer cannot depend on runtime storage");
    }
    return static_constant_declaration(plan, context, found, output);
}

/** Prepare borrowed source/declaration records first, then publish one immutable
 * initializer descriptor. Literal bytes are retained only as original syntax.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) forbids literal-driven inference.
 */
bool static_constant_initializer(StaticConstantPlan *plan, const StaticConstantDeclaration *declaration,
    const AstSource *syntax, const StaticConstantInitializer **output) {
    if (!static_constant_dependencies(plan)) { return false; }
    if (output == NULL || !original(plan, declaration, syntax)) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, (SourceSpan){0}, "Invalid original shared initializer identity or receiver");
    }
    for (const StaticConstantInitializer *item = plan->first_initializer; item; item = item->next) {
        if (item->declaration == declaration && item->syntax == syntax) { *output = item; return true; }
    }
    if (plan->initializer_count == SIZE_MAX) {
        return static_constant_fail(plan, STATIC_CONSTANT_OUT_OF_MEMORY, syntax->span, "Shared initializer count exceeds representable storage");
    }
    const StaticConstantDeclaration *local_source = NULL;
    const StaticConstantSource *qualified = NULL;
    if (syntax->kind == AST_VALUE_SOURCE) {
        if (!local(plan, declaration->context, syntax, &local_source)) { return false; }
    } else if (syntax->kind == AST_CONSTANT_SOURCE) {
        if (!static_constant_source(plan, declaration->context, syntax, &qualified)) { return false; }
    } else if (syntax->kind != AST_LITERAL_SOURCE) {
        return static_constant_fail(plan, STATIC_CONSTANT_CONFLICT, syntax->span, "Shared initializer requires a literal or immutable constant source");
    }
    StaticConstantInitializer *item = static_constant_allocate(plan, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (StaticConstantInitializer){.declaration = declaration, .syntax = syntax,
        .local = local_source, .qualified = qualified, .next = plan->first_initializer};
    plan->first_initializer = item;
    plan->initializer_count++;
    *output = item;
    return true;
}
