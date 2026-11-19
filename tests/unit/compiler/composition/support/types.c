/** @file
 * @brief Compare required concrete Type fields with independently resolved ordinary semantic bindings.
 * The fixtures have no Type formals, so complete nominal identity is exactly the
 * original module pointer; zero captures are asserted rather than assumed for
 * arbitrary templates. No production inference operation supplies the oracle.
 */

/** Required scope identity maps to the original independently checked module,
 * never a basename, generated name or layout-compatible native definition. */
static const SemanticType *required_projection_type(const SemanticProgram *program, const ProjectModule *module) {
    for (const SemanticType *type = program->first; type; type = type->next) {
        if (type->module == module) { return type; }
    }
    assert(false);
    return NULL;
}

/** Original binding source intervals distinguish module fields and sibling
 * lexical locals without repairing a name by a later spelling-based lookup. */
static const SemanticSymbol *required_projection_symbol(const SemanticType *type, SourceSpan name) {
    const SemanticSymbol *selected = NULL;
    for (size_t group = 0; group < 2; group++) {
        for (const SemanticSymbol *symbol = group ? type->locals : type->first; symbol; symbol = symbol->next) {
            if (!same_span(symbol->name, name)) { continue; }
            assert(!selected);
            selected = symbol;
        }
    }
    assert(selected && same_type(selected->owner, type));
    return selected;
}

/** Compare independently computed nominal Type, exact storage, lexical role
 * and original declaration rather than accepting a scheduler readiness bit. */
static void compare_required_field(const SemanticType *type, const StaticTypedField *field) {
    const StaticRuntimeField *source = field->definition;
    const SemanticSymbol *symbol = required_projection_symbol(type, source->name);
    const SymbolRole roles[] = {SYMBOL_CONSTANT, SYMBOL_PARAMETER, SYMBOL_RESULT, SYMBOL_STATE};
    assert((size_t)source->role < sizeof(roles) / sizeof(*roles));
    assert(symbol->reference == source->reference && symbol->list == source->list);
    assert(symbol->local == (source->scope != NULL) && symbol->role == roles[source->role]);
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    bool known = static_constraint_value(&frontend.specializations.constraints, field->type, &value, &outcome);
    assert(known && outcome == STATIC_CONSTRAINT_COMPLETE && value->kind == STATIC_TERM_NOMINAL);
    assert(value->category == STATIC_CATEGORY_TYPE && value->as.nominal.capture_count == 0);
    assert(symbol->type && symbol->type->module == value->as.nominal.origin->definition);
    if (source->type_expression) { assert(symbol->declaration == source->statement); }
}

/** The two independently owned engines share only immutable source identities.
 * Check all fields, including shared/input/result/state and foreach locals;
 * duplicate incoming contexts share no runtime storage and need no second scan. */
static void compare_required_types(const SemanticProgram *program) {
    for (const StaticClosureEntry *entry = frontend.specializations.closure.first; entry; entry = entry->next) {
        if (entry->representative != entry) { continue; }
        const SemanticType *type = required_projection_type(program, entry->scope->environment->definition->definition);
        size_t count = type->symbol_count;
        for (const SemanticSymbol *symbol = type->locals; symbol; symbol = symbol->next) { count++; }
        assert(entry->scope->field_count == count);
        for (const StaticTypedField *field = entry->scope->first_field; field; field = field->next) { compare_required_field(type, field); }
    }
}
