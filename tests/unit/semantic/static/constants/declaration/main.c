/** @file
 * @brief Verify exact dependent shared declarations, empty list element Types and preparation refusals.
 */
#include "../common.c"

/** Literal bytes cannot choose an unknown Type; list storage retains its element
 * Type separately. Repeated binding reuses its exact identity without allocation.
 */
static void it_binds_symbolic_shared_types_without_using_literals_as_type_anchors(void) {
    setup_constant_fixture();
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantDeclaration *scalar, *list, *again;
    assert(static_constant_declaration(&constants, context, constant_named(caller_module, "scalar"), &scalar));
    assert(static_constant_declaration(&constants, context, constant_named(caller_module, "empty"), &list));
    assert(!scalar->list && list->list && scalar->type != list->type && caller_module->parameter_count == 2);
    assert(scalar->source->source->prepared == NULL && scalar->source->source->application == NULL && results.context_count == 1);
    const StaticTerm *formal;
    assert(static_term_symbol(&terms, caller_environment, result_named(caller_module, ".type"), &formal));
    assert(scalar->source->source->source->head == formal && list->source->source->source->head == formal);
    const StaticTerm *value = NULL;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, scalar->type, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_PENDING && value == NULL && scalar->syntax->as.declaration.initializer_count == 1);
    size_t before = allocation_count;
    assert(static_constant_declaration(&constants, context, scalar->syntax, &again));
    assert(again == scalar && allocation_count == before && constants.declaration_count == 2);
    destroy_constants();
}

/** Applied owners substitute original parameters; publishing the Type still
 * executes no initializer and creates no default runtime instance.
 */
static void it_resolves_shared_types_in_their_actual_applied_owner_environment(void) {
    setup_constant_fixture();
    const StaticTermEnvironment *environment = constant_applied_environment();
    const StaticResultContext *context = publication_context(environment);
    const StaticConstantDeclaration *record;
    assert(static_constant_declaration(&constants, context, constant_named(caller_module, "scalar"), &record));
    assert(record->source->source->source->head == integer_type);
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, record->syntax->as.declaration.type, &publication));
    assert(publication->value == integer_type && publications.alias_count == 0);
    assert(record->syntax->as.declaration.initializer_count == 1 && caller_module->parameter_count == 2);
    destroy_constants();
}

/** A Type first introduced by the runtime result interface is visible to the
 * earlier shared section. Empty lists introduce no placeholder or element value.
 */
static void it_binds_result_introduced_parameters_in_earlier_empty_shared_lists(void) {
    setup_constants("@=\nempty[]\n.element\n@>\nreturned&\n.element\n@!\nanchor&\n//integer\n", "@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantDeclaration *generic, *applied;
    const AstNode *syntax = constant_named(caller_module, "empty");
    assert(static_constant_declaration(&constants, context, syntax, &generic));
    assert(generic->list && syntax->as.declaration.first == NULL && caller_module->parameter_count == 1);
    const StaticSymbol *symbol = result_named(caller_module, ".element");
    assert(symbol->role == STATIC_PARAMETER && generic->source->source->source->head->as.variable.symbol == symbol);
    StaticTermCapture capture = {.formal = symbol, .value = integer_type};
    const StaticTerm *owner;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, caller_module, &capture, 1, (SourceSpan){0}, &owner));
    assert(static_term_environment(&terms, caller_module, owner, &environment));
    context = publication_context(environment);
    assert(static_constant_declaration(&constants, context, syntax, &applied));
    assert(applied != generic && applied->type != generic->type && applied->source->source->source->head == integer_type);
    destroy_constants();
}

/** Exported Types supply scalar and empty-list element metadata without local
 * placeholders, implicit instances or inference from literal bytes.
 */
static void it_binds_independently_exported_shared_scalar_and_empty_list_types(void) {
    setup_constants("@=\nscalar\n/test/fixture/v1.0.0/src/other\n .out\n |7\nempty[]\n/test/fixture/v1.0.0/src/other\n .out\n@!\n",
        "@...\n.out\n//integer\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantDeclaration *scalar, *list;
    bool success = static_constant_declaration(&constants, context, constant_named(caller_module, "scalar"), &scalar);
    assert(success);
    success = static_constant_declaration(&constants, context, constant_named(caller_module, "empty"), &list);
    assert(success && !scalar->list && list->list && scalar->type != list->type);
    assert(caller_module->parameter_count == 0 && caller_module->first == NULL);
    const StaticTerm *scalar_type, *element_type;
    success = static_ordinary_resolve(&publications, scalar->source, &scalar_type);
    assert(success);
    success = static_ordinary_resolve(&publications, list->source, &element_type);
    assert(success && scalar_type == integer_type && element_type == scalar_type);
    assert(scalar->source->publication != list->source->publication);
    assert(scalar->source->publication->value == integer_type && list->source->publication->value == integer_type);
    assert(scalar->source->source->producer->registry_use->candidate ==
        list->source->source->producer->registry_use->candidate);
    assert(list->syntax->as.declaration.initializer_count == 0 && publications.alias_count == 0);
    destroy_constants();
}

/** Mutable-object references cannot become shared scalar or list storage. */
static void it_rejects_shared_reference_and_reference_list_storage_before_type_work(void) {
    const char *callers[] = {"@=\nforbidden&\n//integer\n@!\nanchor&\n//integer\n",
        "@=\nforbidden&[]\n//integer\n@!\nanchor&\n//integer\n"};
    for (size_t mode = 0; mode < 2; mode++) {
        setup_constants(callers[mode], "@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticConstantDeclaration *record = NULL;
        assert(!static_constant_declaration(&constants, context, constant_named(caller_module, "forbidden"), &record));
        assert(record == NULL && constants.status == STATIC_CONSTANT_CONFLICT && constants.declaration_count == 0);
        assert(results.use_count == 0 && diagnostic_valid(&constants.error));
        destroy_constants();
    }
}

/** Foreign contexts, copied declarations, wrong sections and wrapped counters
 * cannot publish a shared identity. Dependency errors remain first errors.
 */
static void it_refuses_foreign_shared_identities_and_declaration_counter_wrap(void) {
    for (size_t mode = 0; mode < 5; mode++) {
        setup_constant_fixture();
        const StaticResultContext *context = publication_context(caller_environment);
        StaticResultContext copied_context = *context;
        const AstNode *syntax = constant_named(caller_module, "scalar");
        AstNode copied = *syntax;
        const StaticConstantDeclaration *record = NULL;
        if (mode == 4) { constants.declaration_count = SIZE_MAX; }
        assert(!static_constant_declaration(&constants, mode == 0 ? &copied_context : context,
            mode == 1 ? &copied : mode == 2 ? caller_module->definition->syntax.body.first : syntax,
            mode == 3 ? NULL : &record));
        assert(record == NULL && constants.status == (mode == 4 ? STATIC_CONSTANT_OUT_OF_MEMORY : STATIC_CONSTANT_INVALID));
        const char *message = constants.error.message;
        assert(!static_constant_declaration(&constants, context, syntax, &record) && constants.error.message == message);
        destroy_constants();
    }
    setup_results();
    static_publication_init(&publications, &results, sources.allocator);
    static_constant_init(&constants, &publications, sources.allocator);
    const StaticResultContext *context = publication_context(result_applied_environment());
    const StaticConstantDeclaration *record = NULL;
    assert(!static_publication_resolve(&publications, context, result_pair(".first"), NULL));
    assert(!static_constant_declaration(&constants, context, NULL, &record));
    assert(record == NULL && constants.status == STATIC_CONSTANT_INVALID && constants.error.message == publications.error.message);
    destroy_constants();
}

/** Refuse every observed generic/applied source, Type-metadata, requirement and
 * descriptor allocation, preserving borrowed owners and untouched output handles.
 */
static void it_unwinds_every_dependent_shared_type_preparation_allocation_refusal(void) {
    for (size_t mode = 0; mode < 2; mode++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_constant_fixture();
            const StaticTermEnvironment *environment = mode ? constant_applied_environment() : caller_environment;
            const StaticResultContext *context = publication_context(environment);
            const AstNode *syntax = constant_named(caller_module, "scalar");
            const StaticConstantDeclaration *record = NULL;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = static_constant_declaration(&constants, context, syntax, &record);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else { assert(!success && record == NULL && constants.status == STATIC_CONSTANT_OUT_OF_MEMORY); }
            failure_at = 0;
            destroy_constants();
        }
    }
}

/** Execute shared Type scope, empty lists, storage and resource boundaries. */
static void run(void) {
    it_binds_symbolic_shared_types_without_using_literals_as_type_anchors();
    it_resolves_shared_types_in_their_actual_applied_owner_environment();
    it_binds_result_introduced_parameters_in_earlier_empty_shared_lists();
    it_binds_independently_exported_shared_scalar_and_empty_list_types();
    it_rejects_shared_reference_and_reference_list_storage_before_type_work();
    it_refuses_foreign_shared_identities_and_declaration_counter_wrap();
    it_unwinds_every_dependent_shared_type_preparation_allocation_refusal();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
