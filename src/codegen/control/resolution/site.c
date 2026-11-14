/**
 * @file
 * @brief Resolve unique binding/field operation sites by explicit typed-IR identity.
 *
 * [LIST-04](../../../../spec/language/lists.md#list-04) stages intrinsic literals at their own typed input sites, never module fields.
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) requires exact
 * algorithms: list order, similar names and acquisition-site fallbacks cannot
 * repair missing or duplicate required operations.
 */
#include "site.h"

/** Borrow one exact operation/statement/binding-or-field predicate for a scan. */
typedef struct {
    ControlOperation operation;
    const SemanticStatement *statement;
    const SemanticBinding *binding;
    const SemanticSymbol *field;
} SiteQuery;

/** Scan the complete finite node list, preserving uniqueness independently of order. */
static ControlSiteStatus find(const ControlNode *nodes, const SiteQuery *query, const ControlNode **result) {
    *result = NULL;
    for (const ControlNode *node = nodes; node != NULL; node = node->allocated_next) {
        if (node->operation != query->operation || node->statement != query->statement
            || (query->binding != NULL ? node->binding != query->binding : node->field != query->field)) { continue; }
        if (*result != NULL) {
            *result = NULL;
            return CONTROL_SITE_DUPLICATE;
        }
        *result = node;
    }
    return *result == NULL ? CONTROL_SITE_MISSING : CONTROL_SITE_FOUND;
}

ControlSiteStatus codegen_control_binding_site(const ControlNode *nodes, const SemanticStatement *statement,
    const SemanticBinding *binding, const ControlNode **result) {
    if (binding == NULL) { *result = NULL; return CONTROL_SITE_MISSING; }
    ControlOperation literal = statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION
        ? CONTROL_STAGE_METHOD_INPUT : CONTROL_INSTALL_PARAMETER;
    SiteQuery query = {.operation = binding->transfer == TRANSFER_LITERAL ? literal : CONTROL_EVALUATE_INPUT,
        .statement = statement, .binding = binding};
    return find(nodes, &query, result);
}

ControlSiteStatus codegen_control_field_site(const ControlNode *nodes, const SemanticStatement *statement,
    const SemanticSymbol *field, const ControlNode **result) {
    if (field == NULL || (field->role != SYMBOL_PARAMETER && field->role != SYMBOL_RESULT)) {
        *result = NULL;
        return CONTROL_SITE_MISSING;
    }
    SiteQuery query = {.operation = field->role == SYMBOL_PARAMETER ? CONTROL_INSTALL_PARAMETER : CONTROL_INITIALIZE_RESULT,
        .statement = statement, .field = field};
    return find(nodes, &query, result);
}
