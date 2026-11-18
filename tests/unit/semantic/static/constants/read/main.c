/** @file
 * @brief Verify deferred constant reads retain original namespace and saved-read syntax without member inference.
 */
#include "../common.c"

/** An unknown family remains unknown despite requested members and receivers;
 * static input/output actions remain separate from ordinary saved-read positions.
 */
static void it_registers_unknown_namespaces_without_using_requested_members_as_anchors(void) {
    setup_constants("@<\n...family\n ...slot\n@!\nanchor&\n..integer\n...family\n ...slot < ..integer\n ...out > ...type alias\n constant >> receiver\n", result_target_source);
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantRead *read;
    assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
    assert(read->requests->pending && read->requests->interface == NULL && read->requests->input_count == 0);
    assert(read->requests->output_count == 1 && read->requests->first->slot == NULL);
    assert(read->source->source->head->as.variable.symbol == result_named(caller_module, "...family"));
    assert(read->source->prepared == NULL && read->source->application == NULL && publications.count == 0);
    assert(solver.equation_count == 0 && solver.requirement_count == 0 && caller_module->parameter_count == 1);
    size_t before = allocation_count;
    const StaticConstantRead *again;
    assert(static_constant_read(&constants, context, read->syntax, &again));
    assert(again == read && allocation_count == before && constants.read_count == 1);
    destroy_constants();
}

/** Named/positional requests retain original own sequence positions while their
 * slots remain unresolved, including a list label's exact storage form.
 */
static void it_preserves_pending_named_and_positional_saved_read_shapes(void) {
    const char *callers[] = {
        "@!\nanchor&\n..integer\n.other\n scalar >> x\n list[] >> y[]\n",
        "@!\nanchor&\n..integer\n.other\n >> x\n >> y[]\n"
    };
    for (size_t mode = 0; mode < 2; mode++) {
        setup_constants(callers[mode], "@=\nscalar\n..integer\nlist[]\n..integer\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticConstantRead *read;
        assert(static_constant_read(&constants, context, constant_namespace_read(0), &read));
        assert(read->requests->mode == (mode ? BINDING_MODE_POSITIONAL : BINDING_MODE_NAMED));
        assert(read->requests->output_count == 2 && read->requests->first->source_ordinal == 0);
        assert(read->requests->last->source_ordinal == 1 && read->requests->first->slot == NULL);
        assert(read->requests->last->slot == NULL && (mode || read->requests->last->list));
        assert(constants.declaration_count == 0 && solver.equation_count == 0);
        destroy_constants();
    }
}

/** Foreign/copied contexts and call payloads cannot impersonate namespace sites;
 * checked count growth and first dependency failures remain explicit.
 */
static void it_refuses_foreign_read_handles_counter_wrap_and_borrowed_failures(void) {
    for (size_t mode = 0; mode < 4; mode++) {
        setup_constants("@!\nanchor&\n..integer\n.other\n value >> x\n", "@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        StaticResultContext copied_context = *context;
        const AstNode *syntax = constant_namespace_read(0);
        AstNode copied = *syntax;
        const StaticConstantRead *read = NULL;
        if (mode == 3) { constants.read_count = SIZE_MAX; }
        assert(!static_constant_read(&constants, mode == 0 ? &copied_context : context,
            mode == 1 ? &copied : syntax, mode == 2 ? NULL : &read));
        assert(read == NULL && constants.status == (mode == 3 ? STATIC_CONSTANT_OUT_OF_MEMORY : STATIC_CONSTANT_INVALID));
        destroy_constants();
    }
    setup_constant_fixture();
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantRead *read = NULL;
    assert(!static_constant_read(&constants, context, constant_named(caller_module, "scalar"), &read));
    assert(read == NULL && constants.status == STATIC_CONSTANT_INVALID);
    destroy_constants();
    setup_results();
    static_publication_init(&publications, &results, sources.allocator);
    static_constant_init(&constants, &publications, sources.allocator);
    context = publication_context(result_applied_environment());
    assert(!static_publication_resolve(&publications, context, result_field("first"), NULL));
    assert(!static_constant_read(&constants, context, NULL, &read));
    assert(constants.error.message == publications.error.message && read == NULL);
    destroy_constants();
}

/** Every observed direct-path and unknown-family registration allocation may
 * fail; raw requests never become selected members on a failed preparation.
 */
static void it_unwinds_every_deferred_namespace_registration_allocation_refusal(void) {
    for (size_t mode = 0; mode < 2; mode++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_constants(mode ? "@<\n...family\n ...slot\n@!\nanchor&\n..integer\n...family\n ...slot < ..integer\n ...out > ...alias\n value >> receiver\n" :
                "@!\nanchor&\n..integer\n.other\n value >> receiver\n", result_target_source);
            const StaticResultContext *context = publication_context(caller_environment);
            const AstNode *syntax = constant_namespace_read(0);
            const StaticConstantRead *read = NULL;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = static_constant_read(&constants, context, syntax, &read);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else { assert(!success && read == NULL && constants.status == STATIC_CONSTANT_OUT_OF_MEMORY); }
            failure_at = 0;
            destroy_constants();
        }
    }
}

/** Execute deferred read identity, syntax and resource-boundary evidence. */
static void run(void) {
    it_registers_unknown_namespaces_without_using_requested_members_as_anchors();
    it_preserves_pending_named_and_positional_saved_read_shapes();
    it_refuses_foreign_read_handles_counter_wrap_and_borrowed_failures();
    it_unwinds_every_deferred_namespace_registration_allocation_refusal();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
