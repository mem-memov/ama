/** @file
 * @brief Verify persistent static work identities retain exact required and definition ancestry.
 */
#include "../common.c"

/** Register only checked roots; syntax/value identity is shared independently
 * of actual incoming creation and definition provenance.
 */
static void it_preserves_scope_use_and_definition_ancestry_with_idempotent_requests(void) {
    setup_results();
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, caller_environment, &root));
    const StaticResultContext *context, *again_context;
    assert(static_result_context(&results, root, &context));
    assert(static_result_context(&results, root, &again_context) && context == again_context);
    StaticResultUse *first, *again;
    assert(static_result_use(&results, context, result_projection(0), NULL, &first));
    size_t before = allocation_count;
    assert(static_result_use(&results, context, result_projection(0), NULL, &again));
    assert(first == again && allocation_count == before && first->application == NULL && first->outcome == STATIC_RESULT_PENDING);
    StaticResultDefinitionUse *definition, *again_definition;
    const StaticSymbol *symbol = result_named(caller_module, ".first");
    assert(static_result_definition_use(&results, context, symbol, NULL, (SourceSpan){0}, &definition));
    assert(static_result_definition_use(&results, context, symbol, NULL, (SourceSpan){0}, &again_definition));
    assert(definition == again_definition && definition->context == context && definition->path->creation == root);
    StaticResultUse *plain, *rhs;
    assert(static_result_use(&results, context, definition->definition->source->expression, NULL, &plain));
    assert(static_result_use(&results, context, definition->definition->source->expression, definition->path, &rhs));
    assert(plain != rhs && plain->source == rhs->source && plain->source->value == rhs->source->value);
    assert(rhs->definition_path == definition->path && definition->rhs == NULL && !definition->active);
    const StaticTermEnvironment *applied = result_applied_environment();
    const StaticCreationPath *applied_root;
    const StaticResultContext *applied_context;
    assert(static_creation_root(&guards, applied, &applied_root));
    assert(static_result_context(&results, applied_root, &applied_context));
    assert(applied_context != context && results.context_count == 2 && results.use_count == 3);
    assert(solver.equation_count == 0 && caller_module->parameter_count == 2);
    destroy_results();
}

/** A previously interned definition occurrence cannot bypass the guard when
 * the original source definition requests itself through its RHS.
 */
static void it_checks_definition_ancestry_before_returning_existing_work_records(void) {
    setup_guards("@...\n.a\n.a\n.anchor\n//integer\n", "@!\n");
    initialize_results();
    const StaticCreationPath *root;
    const StaticResultContext *context;
    assert(static_creation_root(&guards, caller_environment, &root));
    assert(static_result_context(&results, root, &context));
    const StaticSymbol *symbol = result_named(caller_module, ".a");
    StaticResultDefinitionUse *first, *output = NULL;
    assert(static_result_definition_use(&results, context, symbol, NULL, (SourceSpan){0}, &first));
    assert(!static_result_definition_use(&results, context, symbol, first->path,
        first->definition->source->expression->span, &output));
    assert(output == NULL && results.definition_use_count == 1 && results.status == STATIC_RESULT_CONFLICT);
    assert(results.error.message == guards.error.message && guards.repeated_definition_ancestor == first->path);
    assert(diagnostic_valid(&results.error));
    destroy_results();
}

/** Exact handles, lexical environments and checked counters determine validity;
 * payload copies cannot be used as an implicit scope or definition path.
 */
static void it_refuses_foreign_contexts_definition_paths_receivers_and_counter_wrap(void) {
    for (size_t mode = 0; mode < 7; mode++) {
        setup_results();
        const StaticCreationPath *root;
        const StaticResultContext *context;
        assert(static_creation_root(&guards, caller_environment, &root));
        assert(static_result_context(&results, root, &context));
        StaticCreationPath copied_root = *root;
        StaticResultContext copied_context = *context;
        const StaticResultContext *context_output = NULL;
        StaticResultUse *use = NULL;
        StaticResultDefinitionUse *definition = NULL;
        const StaticResultSource *source;
        assert(static_result_source(&results, caller_environment, result_projection(0), &source));
        if (mode == 3) { results.context_count = SIZE_MAX; }
        if (mode == 4) { results.use_count = SIZE_MAX; }
        if (mode == 5) { results.definition_use_count = SIZE_MAX; }
        bool success;
        if (mode == 0) { success = static_result_context(&results, &copied_root, &context_output); }
        else if (mode == 1) { success = static_result_use(&results, &copied_context, result_projection(0), NULL, &use); }
        else if (mode == 2) { success = static_result_use(&results, context, result_projection(0), NULL, NULL); }
        else if (mode == 3) {
            const StaticTermEnvironment *applied = result_applied_environment();
            const StaticCreationPath *other;
            assert(static_creation_root(&guards, applied, &other));
            success = static_result_context(&results, other, &context_output);
        } else if (mode == 4) { success = static_result_use(&results, context, result_projection(0), NULL, &use); }
        else if (mode == 5) {
            success = static_result_definition_use(&results, context, result_named(caller_module, ".first"), NULL,
                (SourceSpan){0}, &definition);
        } else {
            StaticResultDefinitionUse *known;
            assert(static_result_definition_use(&results, context, result_named(caller_module, ".first"), NULL, (SourceSpan){0}, &known));
            StaticDefinitionPath copied_path = *known->path;
            success = static_result_use(&results, context, known->definition->source->expression, &copied_path, &use);
        }
        assert(!success && context_output == NULL && use == NULL && definition == NULL);
        assert(results.status == ((mode >= 3 && mode <= 5) ? STATIC_RESULT_OUT_OF_MEMORY : STATIC_RESULT_INVALID));
        destroy_results();
    }
}

/** Sweep every observed request through context publication, expression-use
 * preparation and guarded definition-use preparation in fresh identical owners.
 */
static void it_unwinds_every_context_and_persistent_use_allocation_refusal(void) {
    for (size_t mode = 0; mode < 3; mode++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_results();
            const StaticCreationPath *root;
            const StaticResultContext *context = NULL, *context_output = NULL;
            assert(static_creation_root(&guards, caller_environment, &root));
            if (mode != 0) { assert(static_result_context(&results, root, &context)); }
            StaticResultUse *use = NULL;
            StaticResultDefinitionUse *definition = NULL;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = mode == 0 ? static_result_context(&results, root, &context_output) :
                mode == 1 ? static_result_use(&results, context, result_projection(0), NULL, &use) :
                static_result_definition_use(&results, context, result_named(caller_module, ".first"), NULL, (SourceSpan){0}, &definition);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && context_output == NULL && use == NULL && definition == NULL);
                assert(results.status == STATIC_RESULT_OUT_OF_MEMORY && results.use_count == 0 && results.definition_use_count == 0);
            }
            failure_at = 0;
            assert(inventory.valid);
            destroy_results();
        }
    }
}

/** Required-source edges retain exact handles and deduplicate retries. A copied
 * payload or wrapped count cannot manufacture a source/definition dependency.
 */
static void it_retains_exact_required_dependencies_and_refuses_foreign_edges_or_counter_wrap(void) {
    for (size_t mode = 0; mode < 7; mode++) {
        setup_results();
        const StaticCreationPath *root;
        const StaticResultContext *context;
        assert(static_creation_root(&guards, caller_environment, &root));
        assert(static_result_context(&results, root, &context));
        StaticResultUse *parent, *child;
        StaticResultDefinitionUse *definition;
        assert(static_result_use(&results, context, result_projection(0), NULL, &parent));
        assert(static_result_use(&results, context, result_projection(1), NULL, &child));
        assert(static_result_definition_use(&results, context, result_named(caller_module, ".first"),
            NULL, (SourceSpan){0}, &definition));
        StaticResultUse copied_parent = *parent, copied_child = *child;
        StaticResultDefinitionUse copied_definition = *definition;
        if (mode == 0) {
            assert(static_result_dependency(&results, parent, child, NULL));
            assert(static_result_dependency(&results, parent, NULL, definition));
            size_t before = allocation_count;
            assert(static_result_dependency(&results, parent, child, NULL));
            assert(results.dependency_count == 2 && allocation_count == before);
        } else {
            if (mode == 6) { results.dependency_count = SIZE_MAX; }
            assert(!static_result_dependency(&results, mode == 1 ? &copied_parent : parent,
                mode == 2 ? &copied_child : mode == 3 || mode == 5 ? NULL : child,
                mode == 3 ? &copied_definition : mode == 4 ? definition : NULL));
            assert(results.status == (mode == 6 ? STATIC_RESULT_OUT_OF_MEMORY : STATIC_RESULT_INVALID));
            assert(parent->first_dependency == NULL);
        }
        destroy_results();
    }
}

/** Execute stable work identity, guard ordering and preparation refusals. */
static void run(void) {
    it_preserves_scope_use_and_definition_ancestry_with_idempotent_requests();
    it_checks_definition_ancestry_before_returning_existing_work_records();
    it_refuses_foreign_contexts_definition_paths_receivers_and_counter_wrap();
    it_unwinds_every_context_and_persistent_use_allocation_refusal();
    it_retains_exact_required_dependencies_and_refuses_foreign_edges_or_counter_wrap();
}
/** Return success after counted full teardown. */
int main(void) { run(); return 0; }
