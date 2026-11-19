/** @file
 * @brief Intern source-scoped runtime Type holes without fabricating static formal symbols.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) separates inferred field identity from Type equations.
 * [NAME-06](../../../../../spec/language/declarations.md#name-06) retains lexical foreach identity.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) requires source owners to outlive terms.
 */
#include "../internal.h"

/** Catalog pointer membership precedes following caller-supplied node payloads.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) preserves original syntax provenance.
 */
static bool statement(const StaticTermEnvironment *environment, const AstNode *node) {
    for (const StaticStatement *item = environment->definition->first_statement; item; item = item->next) {
        if (item->syntax == node) { return true; }
    }
    return false;
}

/** Only simple assignment/output operands can introduce module fields. Foreach
 * headers instead identify a lexical binding with their own statement identity.
 * This validates syntax ownership, not the separate source-order classification.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) forbids indexed receiver introductions.
 */
static bool receiver(const AstNode *node, const AstAccess *access) {
    if (access == NULL) { return node->kind == AST_FOREACH; }
    bool original = node->kind == AST_ASSIGNMENT && node->as.assignment.receiver == access;
    if (node->kind == AST_CALL) {
        for (const AstArgument *argument = node->as.call.first; argument; argument = argument->next) {
            if (argument->kind != AST_INPUT && argument->receiver == access) { original = true; break; }
        }
    }
    return original && access->index == NULL;
}

/** Type category is known from runtime storage, while its nominal origin remains
 * unknown until independent typed equations supply evidence. All key components
 * are borrowed immutable source identities; no connection-plan pointer escapes.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) permits unknown required Types without rebinding names.
 */
bool static_term_runtime_type(StaticTermStore *store, const StaticTermEnvironment *environment,
    const AstNode *node, const AstAccess *access, const StaticTerm **output) {
    if (store->status != STATIC_TERM_OK) { return false; }
    if (output == NULL || !static_term_has_environment(store, environment) ||
        !statement(environment, node) || !receiver(node, access)) {
        return static_term_invalid(store, (SourceSpan){0}, "Invalid original runtime Type-hole occurrence");
    }
    StaticVariableIdentity identity = {.role = STATIC_VARIABLE_RUNTIME, .environment = environment,
        .declaration = node, .runtime_access = access};
    SourceSpan site = access ? access->span : node->as.traversal.binder;
    return static_term_variable(store, identity, STATIC_CATEGORY_TYPE, NULL, site, output);
}
