/**
 * @file
 * @brief Build the retained explicit declaration inventory in deterministic source order.
 *
 * [NAME-04](../../../spec/language/declarations.md#name-04)
 * Reject duplicate base names across storage forms, including nested selected handlers
 * and traversal bodies. The separate source-order inference pass consumes this explicit
 * inventory before classifying receivers and scoped iterator bindings.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Explicit spelling/type/initializer metadata precedes receiver classification.
 */
#include <assert.h>
#include "semantic/internal.h"

/** Register each explicit declaration once across storage suffix forms, retaining source/role ordinals. Duplicate failure points at the prior declaration.
 * Component contract: [NAME-04](../../../spec/language/declarations.md#name-04), [TYPE-06](../../../spec/language/declarations.md#type-06). */
static bool declare(SemanticProgram *program, SemanticType *type, const AstNode *node, size_t loop_depth, const AstNode *loop, void *context) {
    (void)loop_depth;
    (void)loop;
    /* [COMP-02](../../../spec/implementation/compiler.md#comp-02) keeps
     * unresolved static syntax out of this concrete-only inventory. T184 owns
     * the future specialization composition; ignoring these nodes would
     * incorrectly publish ordinary runtime types for unprocessed programs.
     */
    const AstTypeExpression *expression = node->kind == AST_DECLARATION
        ? node->as.declaration.type
        : node->kind == AST_CALL ? node->as.call.namespace_expression : NULL;
    if (node->kind == AST_STATIC_FORMAL || node->kind == AST_STATIC_RESULT
        || node->kind == AST_STATIC_PROJECTION
        || (expression != NULL && (expression->kind != AST_TYPE_MODULE_PATH || expression->first != NULL))) {
        return semantic_fail(program, node->span,
            "Static syntax requires resolution before the concrete declaration inventory", NULL);
    }
    if (node->kind != AST_DECLARATION) {
        return true;
    }
    SymbolRole role = *(SymbolRole *)context;
    const AstAccess *name = node->as.declaration.name;
    SemanticSymbol *previous = semantic_symbol(type, name->name);
    if (previous != NULL) {
        return semantic_fail(program, name->span,
            "The base name is already declared in this module", previous);
    }
    SemanticSymbol *symbol = semantic_allocate(program, sizeof(*symbol));
    if (symbol == NULL) {
        return false;
    }
    *symbol = (SemanticSymbol){.owner = type, .declaration = node, .role = role,
        .name = name->name, .spelling = name->span, .site = node->span,
        .type_site = node->as.declaration.type->head,
        .initializer_syntax = node->as.declaration.first,
        .initializer_count = node->as.declaration.initializer_count,
        .ordinal = type->symbol_count++, .role_ordinal = type->counts[role]++,
        .reference = name->reference, .list = name->whole_list};
    if (type->last == NULL) {
        type->first = symbol;
    } else {
        type->last->next = symbol;
    }
    type->last = symbol;
    return true;
}

/** Walk constant/parameter/result/body blocks with distinct declaration roles; this inventory does not classify first-receiver inferred declarations.
 * Component contract: [NAME-04](../../../spec/language/declarations.md#name-04), [TYPE-06](../../../spec/language/declarations.md#type-06). */
static bool module_names(SemanticProgram *program, SemanticType *type) {
    const AstModule *syntax = &type->module->syntax;
    AstBlock blocks[] = {syntax->constants, syntax->parameters, syntax->results, syntax->body};
    for (SymbolRole role = SYMBOL_CONSTANT; role <= SYMBOL_STATE; role++) {
        if (!semantic_walk(program, type, blocks[role], declare, &role)) {
            return false;
        }
    }
    return true;
}

SemanticStatus semantic_names(SemanticProgram *program) {
    assert(!program->names_attempted);
    program->names_attempted = true;
    for (const ProjectModule *module = program->project->first; module != NULL; module = module->next) {
        SemanticType *type = semantic_allocate(program, sizeof(*type));
        if (type == NULL) {
            return program->status;
        }
        type->module = module;
        type->executable = module->syntax.sections.kind != MODULE_CONSTANTS;
        abi_table_init(&type->abi, program->arena.allocator);
        if (program->last == NULL) {
            program->first = type;
        } else {
            program->last->next = type;
        }
        program->last = type;
        if (!module_names(program, type)) {
            return program->status;
        }
    }
    program->names_valid = true;
    return SEMANTIC_OK;
}
