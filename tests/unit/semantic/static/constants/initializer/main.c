/** @file
 * @brief Verify local/qualified typed initializer inference, literal noninference and failure ownership.
 */
#include "../common.c"

/** Bind a known original shared declaration and its first source occurrence. */
static const StaticConstantInitializer *initializer(const StaticResultContext *context, const char *name) {
    const StaticConstantDeclaration *declaration;
    const AstNode *node = constant_named(caller_module, name);
    const StaticConstantInitializer *item;
    assert(static_constant_declaration(&constants, context, node, &declaration));
    assert(static_constant_initializer(&constants, declaration, node->as.declaration.first->source, &item));
    return item;
}

/** Typed local/qualified sources infer the same exact formal, while literals
 * have no Type and leave an otherwise unknown declared parameter unresolved.
 */
static void it_infers_from_local_and_qualified_constants_but_never_literal_bytes(void) {
    const char *sources[] = {"known", "/test/fixture/v1.0.0/src/other/value", "|123"};
    for (size_t mode = 0; mode < 3; mode++) {
        char caller[512];
        int size = snprintf(caller, sizeof(caller), "@=\nknown\n//integer\n |1\ninferred\n.slot\n %s\n@<\ninput\n.slot\n@!\nanchor&\n//integer\n", sources[mode]);
        assert(size > 0 && (size_t)size < sizeof(caller));
        setup_constants(caller, "@=\nvalue\n//integer\n |2\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment), *actual = constant_fixed_target_context();
        const StaticConstantInitializer *item = initializer(context, "inferred"), *again;
        assert(static_constant_initializer(&constants, item->declaration, item->syntax, &again) && again == item);
        StaticResultOutcome outcome;
        assert(static_constant_initializer_resolve(&constants, item, actual, &outcome));
        const StaticTerm *value = NULL;
        StaticConstraintOutcome closed;
        assert(static_constraint_value(&solver, item->declaration->type, &value, &closed));
        assert(closed == (mode == 2 ? STATIC_CONSTRAINT_PENDING : STATIC_CONSTRAINT_COMPLETE));
        if (mode != 2) {
            assert(value == integer_type);
            assert(static_constant_initializer_resolve(&constants, item, actual, &outcome) && outcome == STATIC_RESULT_READY);
        } else { assert(outcome == STATIC_RESULT_PENDING && value == NULL && item->local == NULL && item->qualified == NULL); }
        destroy_constants();
    }
}

/** A concrete nested Type comes from a static assembly. Typed copying can
 * infer an ordinary Type parameter and list element without inline constructors.
 */
static void it_infers_nested_exported_list_element_types_from_own_constant_sources(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/inner", "@<\nvalue\n.item\n@!\n");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/assembly", "@...\n.out\n/test/fixture/v1.0.0/src/other\n .slot < /test/fixture/v1.0.0/src/inner\n  .item < //integer\n");
    setup_constants("@=\nknown[]\n/test/fixture/v1.0.0/src/assembly\n .out\ninferred[]\n.slot\n known[]\n"
        "@<\ninput\n.slot\n@!\nanchor&\n//integer\n", "@<\nvalue\n.slot\n@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticConstantInitializer *item = initializer(context, "inferred");
    StaticResultOutcome outcome;
    assert(item->local && item->declaration->list && item->local->list);
    bool success = static_constant_initializer_resolve(&constants, item, NULL, &outcome);
    assert(success);
    success = static_constant_initializer_resolve(&constants, item, NULL, &outcome);
    assert(success && outcome == STATIC_RESULT_READY);
    const StaticTerm *formal, *value;
    StaticConstraintOutcome closed;
    success = static_term_symbol(&terms, caller_environment, application_formal(caller_module, 0), &formal);
    assert(success);
    success = static_constraint_value(&solver, formal, &value, &closed);
    assert(success && closed == STATIC_CONSTRAINT_COMPLETE && value->as.nominal.origin == target_module);
    const StaticTerm *inner = value->as.nominal.first_capture->value;
    assert(inner->as.nominal.origin->definition == project_find(&project, "/test/fixture/v1.0.0/src/inner"));
    assert(inner->as.nominal.first_capture->value == integer_type);
    assert(constants.initializer_count == 1 && constants.declaration_count == 2);
    destroy_constants();
}

/** Neither a typed constant source nor literal payload can complete the inputs
 * of a static supplier selected as the destination's declared Type.
 */
static void it_refuses_initializer_driven_completion_of_a_selected_static_supplier(void) {
    const char *sources[] = {"known", "|7"};
    for (size_t kind = 0; kind < 2; kind++) {
        char caller[512];
        int length = snprintf(caller, sizeof(caller),
            "@=\nknown\n//integer\nbad\n/test/fixture/v1.0.0/src/other\n .out\n %s\n@!\n", sources[kind]);
        assert(length > 0 && (size_t)length < sizeof(caller));
        setup_constants(caller, "@...\n.out\n.slot\n.anchor\n//integer\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticConstantInitializer *item = initializer(context, "bad");
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        bool success = static_constant_initializer_resolve(&constants, item, NULL, &outcome);
        assert(!success && outcome == STATIC_RESULT_READY && constants.status == STATIC_CONSTANT_UNRESOLVED);
        assert(item->declaration->source->publication == NULL && publications.count == 0);
        destroy_constants();
    }
}

/** Runtime names, mutable links, continuations and self cannot supply constant
 * initialization evidence even when their eventual runtime Type could match.
 */
static void it_rejects_runtime_and_mutable_shared_initializer_sources(void) {
    const char *sources[] = {"runtime", "& known", "% known", "&"};
    for (size_t mode = 0; mode < 4; mode++) {
        char caller[512];
        int size = snprintf(caller, sizeof(caller), "@=\nknown\n//integer\n |1\nbad\n//integer\n %s\n@<\nruntime\n//integer\n@!\n", sources[mode]);
        assert(size > 0 && (size_t)size < sizeof(caller));
        setup_constants(caller, "@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const AstNode *node = constant_named(caller_module, "bad");
        const StaticConstantDeclaration *declaration;
        const StaticConstantInitializer *item = NULL;
        assert(static_constant_declaration(&constants, context, node, &declaration));
        assert(!static_constant_initializer(&constants, declaration, node->as.declaration.first->source, &item));
        assert(item == NULL && constants.status == STATIC_CONSTANT_CONFLICT && constants.initializer_count == 0);
        destroy_constants();
    }
}

/** Distinct fixed source/destination Types conflict; literals do not mask that
 * mismatch. Invalid/copy handles and count overflow cannot publish metadata.
 */
static void it_checks_initializer_identity_counter_bounds_and_type_conflicts(void) {
    for (size_t mode = 0; mode < 7; mode++) {
        setup_constants("@=\nknown\n/test/fixture/v1.0.0/src/other\nbad\n//integer\n known\n@!\n", "@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const AstNode *node = constant_named(caller_module, "bad");
        const StaticConstantDeclaration *declaration;
        assert(static_constant_declaration(&constants, context, node, &declaration));
        StaticConstantDeclaration copied = *declaration;
        AstSource syntax = *node->as.declaration.first->source;
        const StaticConstantInitializer *item = NULL;
        if (mode < 4) {
            if (mode == 2) { constants.initializer_count = SIZE_MAX; }
            assert(!static_constant_initializer(&constants, mode == 0 ? &copied : declaration,
                mode == 1 ? &syntax : node->as.declaration.first->source, mode == 3 ? NULL : &item));
            assert(item == NULL && constants.status == (mode == 2 ? STATIC_CONSTANT_OUT_OF_MEMORY : STATIC_CONSTANT_INVALID));
        } else {
            item = initializer(context, "bad");
            StaticConstantInitializer foreign = *item;
            StaticResultOutcome outcome = STATIC_RESULT_READY;
            assert(!static_constant_initializer_resolve(&constants, mode == 4 ? &foreign : item, NULL, mode == 5 ? NULL : &outcome));
            assert(outcome == STATIC_RESULT_READY && constants.status == (mode == 6 ? STATIC_CONSTANT_CONFLICT : STATIC_CONSTANT_INVALID));
        }
        destroy_constants();
    }
}

/** Every allocation in local/qualified/literal registration and actual Type
 * resolution is independently refused; no value constructor or body executes.
 */
static void it_unwinds_every_initializer_registration_and_resolution_allocation_refusal(void) {
    const char *sources[] = {"known", "/test/fixture/v1.0.0/src/other/value", "|1"};
    for (size_t mode = 0; mode < 3; mode++) {
        for (size_t phase = 0; phase < 2; phase++) {
            size_t total = 0;
            for (size_t cut = 0; cut <= total; cut++) {
                char caller[512];
                int size = snprintf(caller, sizeof(caller), "@=\nknown\n//integer\nbad\n.slot\n %s\n@<\ninput\n.slot\n@!\nanchor&\n//integer\n", sources[mode]);
                assert(size > 0 && (size_t)size < sizeof(caller));
                setup_constants(caller, "@=\nvalue\n//integer\n@!\n");
                const StaticResultContext *context = publication_context(caller_environment), *actual = constant_fixed_target_context();
                const AstNode *node = constant_named(caller_module, "bad");
                const StaticConstantDeclaration *declaration;
                const StaticConstantInitializer *item = NULL;
                assert(static_constant_declaration(&constants, context, node, &declaration));
                if (phase) { assert(static_constant_initializer(&constants, declaration, node->as.declaration.first->source, &item)); }
                size_t before = allocation_count;
                if (cut) { failure_at = before + cut; }
                StaticResultOutcome outcome = STATIC_RESULT_READY;
                bool success = phase ? static_constant_initializer_resolve(&constants, item, actual, &outcome) :
                    static_constant_initializer(&constants, declaration, node->as.declaration.first->source, &item);
                if (!cut) { assert(success); total = allocation_count - before; assert(total > 0); }
                else {
                    assert(!success && constants.status == STATIC_CONSTANT_OUT_OF_MEMORY);
                    assert(phase ? outcome == STATIC_RESULT_READY : item == NULL);
                }
                failure_at = 0;
                destroy_constants();
            }
        }
    }
}

/** Run independent Type evidence and strict source/ownership partitions. */
static void run(void) {
    it_infers_from_local_and_qualified_constants_but_never_literal_bytes();
    it_infers_nested_exported_list_element_types_from_own_constant_sources();
    it_refuses_initializer_driven_completion_of_a_selected_static_supplier();
    it_rejects_runtime_and_mutable_shared_initializer_sources();
    it_checks_initializer_identity_counter_bounds_and_type_conflicts();
    it_unwinds_every_initializer_registration_and_resolution_allocation_refusal();
}
/** Return success only after every assertion and counted owner teardown. */
int main(void) { run(); return 0; }
