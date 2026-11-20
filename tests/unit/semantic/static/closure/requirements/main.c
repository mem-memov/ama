/** @file
 * @brief Verify known-Type validation provenance without changing the constructor-creation graph.
 */
#include "../../connections/common.c"

/** A concrete consumer and a distinct known specialization share source origin. */
static void setup_requirements(const StaticCreationPath **consumer, const StaticTerm **other) {
    setup_connections("@<\nvalue\n.slot\n@!\nanchor&\n//integer\ntext anchor&\n//text\nrecursive&\n/test/fixture/v1.0.0/src/main\n", "@!\n");
    StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
    const StaticTerm *owner;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, caller_module, &input, 1, (SourceSpan){0}, &owner));
    assert(static_term_environment(&terms, caller_module, owner, &environment));
    assert(static_creation_root(&guards, environment, consumer));
    const StaticModule *text = static_module_find(&inventory, project_find(&project, "//text"));
    const StaticTerm *text_type;
    assert(text && static_term_nominal(&terms, text, NULL, 0, (SourceSpan){0}, &text_type));
    input.value = text_type;
    assert(static_term_nominal(&terms, caller_module, &input, 1, (SourceSpan){0}, other));
}

/** Reuse of another specialization does not mean recursive constructor creation. */
static void it_attributes_required_types_without_creating_edges_or_selected_roots(void) {
    const StaticCreationPath *consumer, *required;
    const StaticTerm *other;
    setup_requirements(&consumer, &other);
    SourceSpan site = caller_module->first_statement->syntax->span;
    StaticConstraintOutcome outcome;
    assert(static_guard_require_type(&guards, consumer, other, site, &required, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && required->required_from == consumer);
    assert(required->parent == NULL && consumer->required_from == NULL && guards.edge_count == 0);
    assert(required->vertex && consumer->vertex && required->vertex != consumer->vertex);
    const StaticCreationPath *again;
    size_t paths = guards.path_count;
    assert(static_guard_require_type(&guards, consumer, other, site, &again, &outcome));
    assert(again == required && paths == guards.path_count && guards.edge_count == 0);
    StaticRegistryUse *use;
    StaticRegistryKeyState state;
    assert(static_registry_use(&result_registry, required, &use, &state));
    assert(state == STATIC_REGISTRY_KEY_CLOSED && result_registry.dependency_count == 1);
    assert(result_registry.first_dependency->owner->path == consumer && result_registry.first_dependency->target == use);
    assert(!use->candidate->local_complete);
    /* An explicit root for the same value is distinct from an attributed requirement. */
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, required->owner->environment, &root));
    assert(root != required && root->required_from == NULL && root->parent == NULL);
    destroy_connections();
}

/** A required Type's own written applications still face exact creation
 * ancestry checks: same-key recursion closes; changing-key recursion fails. */
static void it_checks_outgoing_creation_from_reused_types(void) {
    for (size_t different = 0; different < 2; different++) {
        const StaticCreationPath *consumer, *required;
        const StaticTerm *other;
        setup_requirements(&consumer, &other);
        SourceSpan site = caller_module->first_statement->syntax->span;
        StaticConstraintOutcome known;
        assert(static_guard_require_type(&guards, consumer, other, site, &required, &known));
        const StaticTerm *argument = different ? integer_type : other->as.nominal.first_capture->value;
        StaticApplicationInput input = {.formal = application_formal(caller_module, 0), .value = argument};
        StaticApplicationContext context;
        assert(static_application_context(&applications, required->owner->environment, result_field("recursive"), &context));
        const StaticApplicationRecord *application;
        assert(static_application_forward(&applications, caller_constructor, &input, 1, context, &application));
        const StaticCreationPath *child = NULL;
        StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
        bool success = static_creation_enter(&guards, required, application, &child, &outcome);
        assert(success == !different);
        if (different) {
            assert(!child && guards.status == STATIC_GUARD_CONFLICT && guards.repeated_ancestor == required);
        } else {
            assert(child && child->vertex == required->vertex && guards.edge_count == 1);
            assert(child->parent == required && child->required_from == NULL);
        }
        destroy_connections();
    }
}

/** Unknown Type evidence does not publish another owner environment or path. */
static void it_suspends_unknown_requirements_without_preparing_a_scope(void) {
    const StaticCreationPath *consumer;
    const StaticTerm *other;
    setup_requirements(&consumer, &other);
    (void)other;
    const StaticTerm *unknown = application_symbol(".slot");
    size_t paths = guards.path_count, environments = terms.environment_count;
    const StaticCreationPath *output = NULL;
    StaticConstraintOutcome outcome;
    SourceSpan site = caller_module->first_statement->syntax->span;
    assert(static_guard_require_type(&guards, consumer, unknown, site, &output, &outcome));
    assert(outcome == STATIC_CONSTRAINT_PENDING && !output);
    assert(paths == guards.path_count && environments == terms.environment_count);
    assert(static_constraint_equate(&solver, unknown, integer_type, site));
    assert(static_guard_require_type(&guards, consumer, unknown, site, &output, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && output && output->required_from == consumer);
    destroy_connections();
}

/** Copied handles, foreign syntax and missing receivers cannot fabricate provenance. */
static void it_refuses_foreign_requirement_provenance_and_template_values(void) {
    for (size_t kind = 0; kind < 6; kind++) {
        const StaticCreationPath *consumer;
        const StaticTerm *other;
        setup_requirements(&consumer, &other);
        StaticCreationPath copied = *consumer;
        SourceSpan site = caller_module->first_statement->syntax->span;
        if (kind == 1) { site = (SourceSpan){target_module->definition->source->text, 0, 0}; }
        const StaticCreationPath *output = NULL;
        StaticConstraintOutcome outcome = STATIC_CONSTRAINT_PENDING;
        if (kind == 5) { guards.path_count = SIZE_MAX; }
        assert(!static_guard_require_type(&guards, kind == 0 ? &copied : consumer,
            kind == 4 ? caller_constructor : other, site, kind == 2 ? NULL : &output, kind == 3 ? NULL : &outcome));
        assert(!output && outcome == STATIC_CONSTRAINT_PENDING);
        assert(guards.status == (kind == 4 ? STATIC_GUARD_CONFLICT : kind == 5 ? STATIC_GUARD_OUT_OF_MEMORY : STATIC_GUARD_INVALID));
        destroy_connections();
    }
}

/** Every guard preparation/normalization allocation refusal preserves outputs
 * and releases its partial metadata with the ordinary dependency owners. */
static void it_unwinds_every_required_type_preparation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        const StaticCreationPath *consumer, *output = NULL;
        const StaticTerm *other;
        setup_requirements(&consumer, &other);
        SourceSpan site = caller_module->first_statement->syntax->span;
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        StaticConstraintOutcome outcome = STATIC_CONSTRAINT_PENDING;
        bool success = static_guard_require_type(&guards, consumer, other, site, &output, &outcome);
        if (!cut) { assert(success && output); total = allocation_count - before; assert(total > 0); }
        else {
            assert(!success && !output && outcome == STATIC_CONSTRAINT_PENDING);
            assert(guards.status == STATIC_GUARD_OUT_OF_MEMORY && diagnostic_valid(&guards.error));
        }
        failure_at = 0;
        destroy_connections();
    }
}

/** Dispatch real guard/registry compositions and resource failure boundaries. */
static void run(void) {
    it_attributes_required_types_without_creating_edges_or_selected_roots();
    it_checks_outgoing_creation_from_reused_types();
    it_suspends_unknown_requirements_without_preparing_a_scope();
    it_refuses_foreign_requirement_provenance_and_template_values();
    it_unwinds_every_required_type_preparation_allocation_refusal();
}
/** Counted fixture teardown precedes successful process completion. */
int main(void) { run(); return 0; }
