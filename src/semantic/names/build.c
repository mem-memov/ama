/**
 * @file
 * @brief Build the retained explicit declaration inventory in deterministic source order.
 *
 * [NAME-04](../../../spec/language/declarations.md#name-04)
 * Reject duplicate base names across storage forms; first-receiver inference and lexical foreach bindings remain T030/T031.
 */
#include <assert.h>
#include "semantic/internal.h"

/** Register each explicit declaration once across storage suffix forms, retaining source/role ordinals. Duplicate failure points at the prior declaration.
 */
static bool declare(SemanticProgram *program, SemanticType *type, const AstNode *node, size_t loop_depth, void *context) {
    (void)loop_depth;
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
 */
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
