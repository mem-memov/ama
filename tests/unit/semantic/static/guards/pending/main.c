/** @file
 * @brief Verify cached predecessor recurrence, independent branches and exact closure without speculative scopes.
 */
#include "support/prepare.c"

/** The physical annotation frame has no factory parent. Its exact branch
 * owner nevertheless retains the earlier incoming factory-to-branch edge.
 */
static void it_suspends_cached_unknown_recurrence_before_publishing_another_scope(void) {
    for (size_t concrete_request = 0; concrete_request < 2; concrete_request++) {
        PendingFixture fixture = setup_pending(false);
        const StaticTerm *input = concrete_request ? integer_type : fixture.input;
        size_t environments = terms.environment_count, paths = guards.path_count, contexts = results.context_count;
        const StaticCreationPath *output = fixture.annotation;
        StaticCreationOutcome outcome = STATIC_CREATION_ENTERED;
        assert(pending_enter(fixture.annotation, fixture.expression, input, &output, &outcome));
        assert(outcome == STATIC_CREATION_PENDING && output == fixture.annotation);
        assert(terms.environment_count == environments && guards.path_count == paths && results.context_count == contexts);
        assert(static_constraint_equate(&solver, fixture.input, integer_type, fixture.expression->span));
        assert(pending_enter(fixture.annotation, fixture.expression, input, &output, &outcome));
        assert(outcome == STATIC_CREATION_ENTERED && output->parent == fixture.annotation);
        assert(output->vertex == fixture.factory->vertex && guards.path_count == paths + 1);
        destroy_results();
    }
}

/** A second branch's original generic environment has different scoped inputs
 * from the first branch's captured factory inputs. No incoming path joins them.
 */
static void it_keeps_independent_unknown_branches_out_of_cached_ancestry(void) {
    PendingFixture fixture = setup_pending(false);
    const StaticTermEnvironment *environment;
    assert(static_term_environment(&terms, fixture.branch->owner->environment->definition, NULL, &environment));
    const StaticCreationPath *annotation;
    assert(static_creation_annotation(&guards, environment, fixture.expression, &annotation));
    assert(annotation->owner != fixture.annotation->owner);
    const StaticTerm *input;
    assert(static_term_symbol(&terms, environment, application_formal(environment->definition, 0), &input));
    assert(input != fixture.input);
    const StaticCreationPath *output = NULL;
    StaticCreationOutcome outcome;
    assert(pending_enter(annotation, fixture.expression, input, &output, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED && output && output->parent == annotation);
    assert(output->owner != fixture.factory->owner);
    destroy_results();
}

/** Complete unequal specializations are rejected through the original cached
 * path even though the new lexical frame has no physical producer ancestor.
 */
static void it_rejects_known_unequal_keys_through_cached_annotation_predecessors(void) {
    PendingFixture fixture = setup_pending(false);
    assert(static_constraint_equate(&solver, fixture.input, integer_type, fixture.expression->span));
    const StaticModule *text = static_module_find(&inventory, project_find(&project, "//text"));
    const StaticTerm *other;
    assert(text && static_term_nominal(&terms, text, NULL, 0, fixture.expression->span, &other));
    assert(other->category == STATIC_CATEGORY_TYPE && other->ground);
    const StaticCreationPath *output = fixture.annotation;
    StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
    size_t contexts = results.context_count;
    assert(!pending_enter(fixture.annotation, fixture.expression, other, &output, &outcome));
    assert(output == fixture.annotation && outcome == STATIC_CREATION_PENDING && results.context_count == contexts);
    assert(guards.status == STATIC_GUARD_CONFLICT && guards.rejected_origin == target_module);
    assert(guards.creation_witness && guards.earlier_creation_key && guards.rejected_creation_key);
    destroy_results();
}

/** Reverse reachability allocates only scratch/normalization storage and
 * releases every failed attempt without introducing a candidate scope or path.
 */
static void it_unwinds_every_cached_predecessor_search_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        PendingFixture fixture = setup_pending(false);
        size_t before = allocation_count, paths = guards.path_count, contexts = results.context_count;
        if (failure) { failure_at = before + failure; }
        bool pending = false;
        bool success = static_guard_cached_pending(&guards, fixture.annotation, fixture.factory->owner, &pending);
        if (!failure) { assert(success && pending); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !pending && guards.status == STATIC_GUARD_OUT_OF_MEMORY); }
        assert(guards.path_count == paths && results.context_count == contexts);
        failure_at = 0;
        destroy_results();
    }
}

/** A closed ordinary owner can supply independent annotation facts even when
 * another incoming factory use still has an unknown input. Its canonical scope
 * is finite; the old unresolved path remains separately required and pending.
 */
static void it_does_not_inherit_unrelated_pending_ancestry_into_closed_metadata_owners(void) {
    PendingFixture fixture = setup_pending(true);
    assert(fixture.annotation->vertex && fixture.factory->vertex == NULL);
    const StaticCreationPath *output = NULL;
    StaticCreationOutcome outcome;
    assert(pending_enter(fixture.annotation, fixture.expression, integer_type, &output, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED && output && output->vertex);
    assert(output->parent == fixture.annotation && fixture.factory->vertex == NULL);
    destroy_results();
}

/** A real intermediate supplier keeps the open annotation boundary in its
 * physical chain. Its repeated descendant must check the cached predecessors
 * too, even though its immediate parent is not itself an annotation frame.
 */
static void it_retains_open_annotation_boundaries_through_actual_supplier_descendants(void) {
    PendingFixture fixture = setup_pending(false);
    const StaticTermEnvironment *environment = fixture.branch->owner->environment;
    const AstTypeExpression *expression = pending_field(environment->definition, "via");
    const StaticCreationPath *annotation, *middle;
    assert(static_creation_annotation(&guards, environment, expression, &annotation));
    const StaticModule *module = static_module_find(&inventory,
        project_find(&project, "/test/fixture/v1.0.0/src/middle"));
    StaticTermCapture input = {.formal = application_formal(module, 0), .value = fixture.input};
    StaticCreationOutcome outcome;
    assert(static_creation_producer_enter(&guards, annotation, expression, &input, 1, &middle, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED && !middle->source_only && middle->parent == annotation);
    const AstTypeExpression *request = NULL;
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->root && !site->receiver && site->statement->kind == AST_STATIC_PROJECTION) { request = site->expression; break; }
    }
    assert(request);
    const StaticCreationPath *output = middle;
    size_t paths = guards.path_count, environments = terms.environment_count;
    assert(pending_enter(middle, request, fixture.input, &output, &outcome));
    assert(outcome == STATIC_CREATION_PENDING && output == middle);
    assert(guards.path_count == paths && terms.environment_count == environments);
    destroy_results();
}

/** Execute cached suspension, exact resolution, branch and allocation partitions. */
static void run(void) {
    it_suspends_cached_unknown_recurrence_before_publishing_another_scope();
    it_keeps_independent_unknown_branches_out_of_cached_ancestry();
    it_rejects_known_unequal_keys_through_cached_annotation_predecessors();
    it_unwinds_every_cached_predecessor_search_allocation_refusal();
    it_does_not_inherit_unrelated_pending_ancestry_into_closed_metadata_owners();
    it_retains_open_annotation_boundaries_through_actual_supplier_descendants();
}
/** Return only after every scoped owner has released its counted storage. */
int main(void) { run(); return 0; }
