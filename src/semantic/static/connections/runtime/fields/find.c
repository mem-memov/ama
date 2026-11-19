/** @file
 * @brief Resolve fixed module declarations and exact foreach-body lexical scopes.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) separates module-wide and lexical storage.
 */
#include "../../internal.h"

/** Module bindings are unique; lexical cells never enter module-wide lookup.
 * [NAME-04](../../../../../../spec/language/declarations.md#name-04) gives all storage suffixes one base name.
 */
const StaticRuntimeField *static_runtime_field(const StaticRuntimeModule *module, SourceSpan name) {
    for (const StaticRuntimeField *field = module->first_field; field; field = field->next) {
        if (field->scope == NULL && static_same_name(field->name, name)) { return field; }
    }
    return NULL;
}

/** Exact body intervals exclude the header, subsequent code and sibling scopes.
 * Nearest lexical containment is determined by the largest opening-marker site.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) forbids dynamic lookup or shadowing.
 */
const StaticRuntimeField *static_runtime_local(const StaticRuntimeModule *module, SourceSpan name, SourceSpan site) {
    const StaticRuntimeField *found = NULL;
    size_t nearest = 0;
    for (const StaticRuntimeField *field = module->first_field; field; field = field->next) {
        if (field->scope == NULL || !static_same_name(field->name, name)) { continue; }
        SourceSpan open = field->scope->as.traversal.body_marker, close = field->scope->as.traversal.closing_marker;
        if (site.source == open.source && site.start >= open.end && site.end <= close.start &&
            (found == NULL || open.start > nearest)) { found = field; nearest = open.start; }
    }
    return found;
}

/** Original AST pointer membership, not a same-spelled name, locates defaults.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) preserves initializer ownership.
 */
const StaticRuntimeField *static_runtime_declaration(const StaticRuntimeModule *module, const AstNode *syntax) {
    for (const StaticRuntimeField *field = module->first_field; field; field = field->next) {
        if (!field->inferred && field->scope == NULL && field->statement == syntax) { return field; }
    }
    return NULL;
}
