/** @file
 * @brief Keep complete typed analysis separate from not-yet-implemented execution adapters.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) permits inspecting
 * resolved IR without executing it. [CALL-09](../../../spec/language/calls-and-results.md#call-09),
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04),
 * [FLOW-05](../../../spec/language/lists.md#flow-05) and
 * [LIFE-02](../../../spec/implementation/native-interface.md#life-02) forbid
 * publishing C that drops selected actions, cursor state or final self identity.
 */
#include "codegen/internal.h"
/** Retain the exact original site and report unsupported execution before creating any artifact. */
static bool unsupported(NativeScaffold *scaffold, SourceSpan site, const char *message) {
    scaffold_error(scaffold, DIAGNOSTIC_UNSUPPORTED_FEATURE, message, NULL);
    scaffold->error.primary.span = site;
    return false;
}
/** A self source is fully typed but still needs final construction/clone identity in the execution adapter. */
static bool source(NativeScaffold *scaffold, const SemanticSource *value) {
    if (value != NULL && value->kind == AST_SELF_REFERENCE_SOURCE) {
        return unsupported(scaffold, value->site, "Standalone self-reference execution awaits final construction and clone identity support");
    }
    return true;
}
bool codegen_validate_execution(NativeScaffold *scaffold) {
    for (const SemanticType *type = scaffold->frontend->semantic.first; type != NULL; type = type->next) {
        for (const SemanticSymbol *field = type->first; field != NULL; field = field->next) {
            for (const SemanticInitializer *value = field->initializers; value != NULL; value = value->next) {
                if (!source(scaffold, value->source)) { return false; }
            }
        }
        for (const SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
            if (!source(scaffold, statement->source)) { return false; }
            for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
                if (!source(scaffold, binding->source)) { return false; }
            }
            const AstNode *node = statement->syntax;
            if (node->kind == AST_FOREACH || (node->kind == AST_LOOP && node->as.control.command != NULL)) {
                return unsupported(scaffold, node->span, "Traversal and condition-chain IR require their execution adapters");
            }
            if (node->kind == AST_CALL && node->as.call.has_selected) {
                return unsupported(scaffold, node->span, "Direct result-selection IR requires its execution adapter");
            }
        }
    }
    return true;
}
