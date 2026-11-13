/**
 * @file
 * @brief Resolve foreach-local references by exact nested lexical body intervals.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) excludes the
 * header, following code, sibling bodies and construction-time uses from new local scope.
 */
#include "../internal.h"
SemanticSymbol *semantic_local_at(const SemanticType *type, SourceSpan name, SourceSpan site) {
    SemanticSymbol *found = NULL;
    size_t nearest = 0;
    for (SemanticSymbol *local = type->locals; local != NULL; local = local->next) {
        const AstNode *scope = local->scope;
        SourceSpan open = scope->as.traversal.body_marker, close = scope->as.traversal.closing_marker;
        if (site.source == open.source && site.start >= open.end && site.end <= close.start
            && semantic_same_text(local->name, name) && (found == NULL || open.start > nearest)) {
            found = local;
            nearest = open.start;
        }
    }
    return found;
}
