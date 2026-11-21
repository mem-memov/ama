/** @file
 * @brief Select qualified own constants in independently checked fixed namespace scopes.
 * [CONST-02](../../../../../spec/language/declarations.md#const-02) fixes member and storage form.
 * [CONST-05](../../../../../spec/language/declarations.md#const-05) forbids open namespaces.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial selection.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"
#include "../../internal.h"

/** Exact live registration identity precedes borrowed source access.
 * [CONST-02](../../../../../spec/language/declarations.md#const-02) prevents fabricated sources.
 */
static bool registered(const StaticConstantPlan *plan, const StaticConstantSource *candidate) {
    for (const StaticConstantSource *source = plan->first_source; source; source = source->next) {
        if (source == candidate) { return true; }
    }
    return false;
}

/** The own surface already rejects duplicate names and forbidden shared storage;
 * compare complete base names, then enforce the written scalar/list suffix.
 * [CONST-02](../../../../../spec/language/declarations.md#const-02) preserves exact selection.
 */
static const AstNode *member(const BindingInterface *view, const ModulePathPart *name) {
    for (const BindingSlot *slot = view->sequences[BINDING_RUNTIME_OUTPUT].first; slot; slot = slot->next) {
        const AstNode *node = slot->runtime_declaration;
        if (node) {
            SourceSpan declared = node->as.declaration.name->name;
            if (declared.end - declared.start == name->length
                && memcmp(source_bytes(declared.source) + declared.start,
                    name->bytes, name->length) == 0) { return node; }
        }
    }
    return NULL;
}

/** Final selection never adds namespace equations or creation edges. Context
 * identity is checked independently; member Type obligations remain scoped to it.
 * Repeated selection revalidates guards and scope before descriptor reuse.
 * [CONST-05](../../../../../spec/language/declarations.md#const-05) requires independent namespace closure.
 */
bool static_constant_value(StaticConstantPlan *plan, const StaticConstantSource *source,
    const StaticResultContext *context, const StaticConstantValue **output) {
    if (!static_constant_dependencies(plan)) { return false; }
    if (output == NULL || !registered(plan, source)) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, (SourceSpan){0}, "Invalid qualified source selection identity or receiver");
    }
    const AstSource *syntax = source->occurrence->source;
    if (source->namespace_type->category != STATIC_CATEGORY_TYPE) {
        return static_constant_fail(plan, STATIC_CONSTANT_UNRESOLVED, syntax->qualified_path, "Qualified namespace needs explicit template arguments");
    }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan->publications->results->guards, &closure)) {
        (void)static_constant_dependencies(plan); return false;
    }
    if (!static_constant_scope(plan, context, source->namespace_type)) { return false; }
    for (const StaticConstantValue *value = plan->first_value; value; value = value->next) {
        if (value->source == source && value->context == context) { *output = value; return true; }
    }
    BindingInterface *view;
    if (!static_constant_surface(plan, source->namespace_type, &view)) { return false; }
    const AstNode *node = member(view, &source->occurrence->path->member);
    if (node == NULL || node->as.declaration.name->whole_list != syntax->constant_list) {
        return static_constant_fail(plan, STATIC_CONSTANT_CONFLICT, syntax->span, "Qualified own constant or storage form does not match its declaration");
    }
    if (plan->value_count == SIZE_MAX) {
        return static_constant_fail(plan, STATIC_CONSTANT_OUT_OF_MEMORY, syntax->span, "Qualified value count exceeds representable storage");
    }
    const StaticConstantDeclaration *declaration;
    if (!static_constant_declaration(plan, context, node, &declaration)) { return false; }
    StaticConstantValue *value = static_constant_allocate(plan, sizeof(*value));
    if (value == NULL) { return false; }
    *value = (StaticConstantValue){.source = source, .context = context, .declaration = declaration, .next = plan->first_value};
    plan->first_value = value;
    plan->value_count++;
    *output = value;
    return true;
}
