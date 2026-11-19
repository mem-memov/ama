/** @file
 * @brief Copy fixed source fields into concrete owners without repeating name inference.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) preserves source classification.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) excludes constants-only namespaces from runtime field Types.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) links complete nominal identities.
 * [NAME-04](../../../../spec/language/declarations.md#name-04) preserves section roles.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) keeps local cells outside persistent storage.
 */
#include <stdint.h>
#include "internal.h"

/** Only normalized complete Type identity selects a target vertex; the original
 * module path is insufficient when the same origin has distinct captures.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) requires the entire key.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) also requires an instance-bearing module.
 */
static SemanticType *target(SemanticProgram *program, StaticConstraintSolver *solver,
    const StaticTypedField *field) {
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    if (!static_constraint_value(solver, field->type, &value, &outcome)) {
        if (solver->status == STATIC_CONSTRAINT_OUT_OF_MEMORY) { semantic_memory_failure(program); }
        else { (void)semantic_fail(program, field->definition->site, "Cannot normalize a required field Type", NULL); }
        return NULL;
    }
    if (outcome == STATIC_CONSTRAINT_COMPLETE && value->kind == STATIC_TERM_NOMINAL
        && value->category == STATIC_CATEGORY_TYPE) {
        for (SemanticType *type = program->first; type; type = type->next) {
            const StaticRegistryCandidate *candidate = type->specialization->context->registry_use->candidate;
            if (candidate->key->owner_type != value) { continue; }
            if (!type->executable) {
                const StaticRuntimeField *definition = field->definition;
                SourceSpan site = definition->type_expression ? definition->type_expression->head : definition->site;
                (void)semantic_fail(program, site, "A value type must name an executable module", NULL);
                return NULL;
            }
            return type;
        }
    }
    (void)semantic_fail(program, field->definition->site,
        "A field requires a complete registered nominal Type", NULL);
    return NULL;
}

/** Map enumerated source section roles explicitly; enum layout is not an ABI.
 * [NAME-04](../../../../spec/language/declarations.md#name-04) preserves each storage role.
 */
static bool role(StaticRuntimeRole source, SymbolRole *output) {
    switch (source) {
        case STATIC_FIELD_CONSTANT: *output = SYMBOL_CONSTANT; return true;
        case STATIC_FIELD_INPUT: *output = SYMBOL_PARAMETER; return true;
        case STATIC_FIELD_RESULT: *output = SYMBOL_RESULT; return true;
        case STATIC_FIELD_STATE: *output = SYMBOL_STATE; return true;
    }
    return false;
}

/** Preserve declaration/receiver spans and defaults, then publish one owned
 * symbol. Local cells have no persistent field or interface ordinal.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes introducing receivers.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) excludes local storage.
 */
static bool field(SemanticProgram *program, StaticConstraintSolver *solver,
    SemanticType *owner, const StaticTypedField *source) {
    const StaticRuntimeField *original = source->definition;
    SymbolRole section;
    if (!role(original->role, &section)) {
        return semantic_fail(program, original->site, "Unknown original runtime field role", NULL);
    }
    bool local = original->scope != NULL;
    if (!local && (owner->symbol_count == SIZE_MAX || owner->counts[section] == SIZE_MAX)) {
        return semantic_memory_failure(program);
    }
    SemanticType *type = target(program, solver, source);
    if (!type) { return false; }
    SemanticSymbol *symbol = semantic_allocate(program, sizeof(*symbol));
    if (!symbol) { return false; }
    const AstNode *declaration = original->type_expression ? original->statement : NULL;
    SourceSpan spelling = declaration ? declaration->as.declaration.name->span
        : original->receiver ? original->receiver->span : original->name;
    if (local) { spelling.end++; }
    *symbol = (SemanticSymbol){.original = original, .declaration = declaration,
        .name = original->name, .spelling = spelling,
        .site = declaration ? declaration->span : spelling,
        .type_site = original->type_expression ? original->type_expression->head : original->site,
        .initializer_syntax = declaration ? declaration->as.declaration.first : NULL,
        .initializer_count = declaration ? declaration->as.declaration.initializer_count : 0,
        .inferred = original->inferred, .local = local, .scope = original->scope,
        .owner = owner, .type = type, .role = section,
        .reference = original->reference, .list = original->list};
    if (local) { symbol->next = owner->locals; owner->locals = symbol; }
    else {
        symbol->ordinal = owner->symbol_count++;
        symbol->role_ordinal = owner->counts[section]++;
        if (owner->last) { owner->last->next = symbol; }
        else { owner->first = symbol; }
        owner->last = symbol;
    }
    return true;
}

bool semantic_specialization_fields(SemanticProgram *program, StaticConstraintSolver *solver,
    SemanticType *type) {
    for (const StaticTypedField *item = type->specialization->scope->first_field; item; item = item->next) {
        if (!field(program, solver, type, item)) { return false; }
    }
    return semantic_specialization_foreach(program, type);
}
