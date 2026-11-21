/** @file
 * @brief Test marker identity and forwarding through the actual checked static graph.
 */
#include "prepare.c"

/** All public graph outputs must close, including markers not selected for runtime storage. */
static void it_forwards_distinct_marker_types_without_runtime_layout(void) {
    prepare_marker_project("@!\nvalue\n./assembly\n .value\n");
    setup_frontend();
    bool checked = check_project();
    if (!checked) { fprintf(stderr, "Marker frontend: %s\n", frontend.error.message); }
    assert(checked);
    const StaticModule *node = static_module_find(&frontend.static_inventory,
        project_find(&frontend.project, "/test/fixture/v1.0.0/src/node"));
    const StaticModule *edge = static_module_find(&frontend.static_inventory,
        project_find(&frontend.project, "/test/fixture/v1.0.0/src/edge"));
    assert(node && edge && node != edge && node->parameter_count == 0 && node->result_count == 0);
    assert(node->definition->syntax.sections.kind == MODULE_MARKER);
    const StaticTerm *first, *second, *same;
    StaticTermStore *terms = &frontend.specializations.terms;
    assert(static_term_nominal(terms, node, NULL, 0, (SourceSpan){0}, &first));
    assert(static_term_nominal(terms, edge, NULL, 0, (SourceSpan){0}, &second));
    assert(static_term_nominal(terms, node, NULL, 0, (SourceSpan){0}, &same));
    assert(first == same && first != second && first->category == STATIC_CATEGORY_TYPE && first->ground);
    size_t forwarded = 0;
    for (const StaticPublication *publication = frontend.specializations.publications.first;
        publication; publication = publication->next) {
        for (const StaticPublishedOutput *output = publication->first_output; output; output = output->next) {
            if (layout_text_is(output->symbol->name, ".node kind")
                || layout_text_is(output->symbol->name, ".same node kind")
                || layout_text_is(output->symbol->name, ".forwarded")) {
                assert(output->value == first);
                forwarded++;
            }
        }
    }
    assert(forwarded == 3);
    for (const StaticResultContext *context = frontend.specializations.results.first_context;
        context; context = context->next) {
        assert(context->creation->owner->environment->definition != node);
        assert(context->creation->owner->environment->definition != edge);
    }
    for (const SemanticType *type = frontend.semantic.first; type; type = type->next) {
        assert(type->module != node->definition && type->module != edge->definition);
    }
    destroy_frontend();
}

/** Standard and ordinary roots retain different nominal keys despite equal final names. */
static void it_preserves_root_kind_in_marker_identity(void) {
    prepare_marker_project("@!\nvalue\n./assembly\n .value\n");
    directory("generated/standard");
    file("generated/standard/node", "@...\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly",
        "@...\n.ordinary\n./node\n.standard\n//node\n.value\n./value\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    assert(compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    const StaticTerm *ordinary = NULL, *standard = NULL;
    for (const StaticPublication *publication = frontend.specializations.publications.first;
        publication; publication = publication->next) {
        for (const StaticPublishedOutput *output = publication->first_output; output; output = output->next) {
            if (layout_text_is(output->symbol->name, ".ordinary")) { ordinary = output->value; }
            if (layout_text_is(output->symbol->name, ".standard")) { standard = output->value; }
        }
    }
    assert(ordinary && standard && ordinary != standard);
    destroy_frontend();
}

/** Public runtime binding APIs reject markers even when called independently
 * of the compiler's earlier root and creation classifications.
 */
static void it_refuses_marker_runtime_binding_and_scope_handles(void) {
    prepare_marker_project("@!\nvalue\n./assembly\n .value\n");
    setup_frontend();
    assert(check_project());
    const StaticModule *node = static_module_find(&frontend.static_inventory,
        project_find(&frontend.project, "/test/fixture/v1.0.0/src/node"));
    StaticConnectionPlan isolated;
    static_connection_init(&isolated, &frontend.specializations.constants, sources.allocator);
    const StaticRuntimeModule *runtime = NULL;
    assert(!static_connection_runtime(&isolated, node, &runtime));
    assert(runtime == NULL && isolated.status == STATIC_CONNECTION_INVALID && isolated.runtime_count == 0);
    static_connection_destroy(&isolated);
    const StaticTerm *marker;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&frontend.specializations.terms, node, NULL, 0, (SourceSpan){0}, &marker));
    assert(static_term_environment(&frontend.specializations.terms, node, marker, &environment));
    static_connection_init(&isolated, &frontend.specializations.constants, sources.allocator);
    const StaticTypedScope *scope = NULL;
    assert(!static_connection_scope(&isolated, environment, &scope));
    assert(scope == NULL && isolated.status == STATIC_CONNECTION_INVALID && isolated.scope_count == 0);
    static_connection_destroy(&isolated);
    destroy_frontend();
}

/** Every failed prepare remains destructible; no failed allocation may masquerade as a marker value. */
static void it_unwinds_every_marker_frontend_allocation_refusal(void) {
    prepare_marker_project("@!\nvalue\n./assembly\n .value\n");
    setup_frontend();
    assert(check_project());
    size_t total = allocation_count;
    destroy_frontend();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_frontend();
        failure_at = failure;
        assert(!check_project());
        assert(!frontend.valid);
        failure_at = 0;
        destroy_frontend();
    }
}
