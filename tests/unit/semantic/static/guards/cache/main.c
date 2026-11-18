/** @file
 * @brief Compose concrete creation caching with lexical provenance and independent symbolic anchors.
 */
#include "../common.c"
#include "semantic/static/bindings/normalize.h"

/** A and B apply their received one-slot family to their own element Type. */
static const char *family_source = "@<\n...element\n...next\n ...element\n@!\n"
    "anchor&\n..integer\nchild&\n...next\n ...element < ...element\n"
    "leaf&\n.leaf\n ...element < ..integer\n";
static const char *leaf_source = "@<\n...element\n@!\n";
static const char *returning_leaf_source = "@<\n...element\n@!\nback&\n.main\n"
    " ...element < ...element\n ...next < .leaf\n";

/** Find original root declaration syntax in its actual lexical module. */
static const AstTypeExpression *field(const StaticModule *module, const char *name) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, name)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** The third real source constructor makes the finite tails in both independent
 * specialization paths explicit; no native module is treated as a template.
 */
static const StaticTerm *setup_families(bool returning) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/leaf", returning ? returning_leaf_source : leaf_source);
    setup_guards(family_source, family_source);
    const StaticModule *leaf = static_module_find(&inventory, project_find(&project, ".leaf"));
    assert(leaf && leaf->parameter_count == 1);
    const StaticTerm *constructor;
    assert(static_term_nominal(&terms, leaf, NULL, 0, (SourceSpan){0}, &constructor));
    return constructor;
}

/** Fix only next; the original element slot stays a local residual. */
static const StaticTerm *family(const StaticModule *module, const StaticTerm *next) {
    StaticTermCapture captured = {.formal = application_formal(module, 1), .value = next};
    const StaticTerm *value;
    assert(static_term_nominal(&terms, module, &captured, 1, (SourceSpan){0}, &value));
    return value;
}

/** Select one complete root without changing any source definition. */
static const StaticCreationPath *root(const StaticModule *module, const StaticTerm *next) {
    StaticTermCapture captures[] = {{.formal = application_formal(module, 0), .value = integer_type},
        {.formal = application_formal(module, 1), .value = next}};
    const StaticTerm *value;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, module, captures, 2, (SourceSpan){0}, &value));
    assert(static_term_environment(&terms, module, value, &environment));
    const StaticCreationPath *path;
    assert(static_creation_root(&guards, environment, &path));
    return path;
}

/** Evaluate this fixture's explicit next(element) source handoff using its
 * original owner bindings. T173 reselects the actual residual input after capture.
 */
static const StaticApplicationRecord *child_application(const StaticCreationPath *parent) {
    const StaticTermEnvironment *environment = parent->owner->environment;
    const StaticModule *module = environment->definition;
    const StaticTerm *head, *element;
    assert(static_term_symbol(&terms, environment, application_formal(module, 1), &head));
    assert(static_term_symbol(&terms, environment, application_formal(module, 0), &element));
    const AstTypeExpression *expression = field(module, "child");
    BindingApplication *requests;
    assert(binding_normalize_static(&binding_plan, NULL, expression, &requests));
    StaticTermOperand input = {.request = requests->first, .value = element};
    const StaticTermApplication *source;
    assert(static_term_application(&terms, &binding_plan, requests, environment, head, &input, 1, &source));
    const StaticApplicationRecord *record;
    assert(static_application_source(&applications, source, &record));
    return record;
}

/** Follow one checked lexical creation edge after the actual source handoff. */
static const StaticCreationPath *child(const StaticCreationPath *parent) {
    const StaticApplicationRecord *record = child_application(parent);
    const StaticCreationPath *path;
    StaticCreationOutcome outcome;
    assert(static_creation_enter(&guards, parent, record, &path, &outcome) && outcome == STATIC_CREATION_ENTERED);
    return path;
}

/** An actual captured constructor retains its original source even when the
 * next concrete application would terminate. TPL-11 rejects this source repeat
 * before expansion, independently of the received formal's descriptive name.
 */
static void it_rejects_a_captured_own_constructor_even_with_finite_next_captures(void) {
    const StaticTerm *leaf = setup_families(false);
    const StaticCreationPath *owner = root(caller_module, family(caller_module, leaf));
    const StaticApplicationRecord *record = child_application(owner);
    assert(record->constructor->as.nominal.origin == caller_module);
    assert(record->constructor->as.nominal.capture_count == 1 && record->value->ground);
    assert(record->value != owner->owner->value);
    const StaticCreationPath *output = NULL;
    StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
    assert(!static_creation_enter(&guards, owner, record, &output, &outcome));
    assert(output == NULL && outcome == STATIC_CREATION_PENDING);
    assert(guards.status == STATIC_GUARD_CONFLICT && guards.repeated_ancestor == owner);
    assert(guards.rejected_origin == caller_module && guards.path_count == 1);
    destroy_guards();
}

/** A(B(Leaf)) and B(A(Leaf)) have distinct concrete A/B vertices. Their source
 * names alone would form a fictitious cycle; both true paths end in Leaf.
 * These are guard-component checks, not admission of unexpanded source fields.
 */
static void it_preserves_distinct_specializations_and_shared_finite_tails(void) {
    const StaticTerm *leaf = setup_families(false);
    const StaticCreationPath *a = root(caller_module, family(target_module, leaf));
    const StaticCreationPath *b_child = child(a), *first_leaf = child(b_child);
    const StaticCreationPath *b = root(target_module, family(caller_module, leaf));
    const StaticCreationPath *a_child = child(b), *second_leaf = child(a_child);
    assert(a->vertex != a_child->vertex && b->vertex != b_child->vertex);
    assert(first_leaf->vertex == second_leaf->vertex && first_leaf != second_leaf);
    assert(guards.vertex_count == 5 && guards.edge_count == 4 && guards.path_count == 6);
    assert(first_leaf->edge != second_leaf->edge && first_leaf->application != second_leaf->application);
    size_t before = allocation_count, live = live_allocations;
    for (size_t repeat = 0; repeat < 100; repeat++) {
        StaticGuardClosure closure;
        assert(static_guard_refresh(&guards, &closure) && closure == STATIC_GUARD_CLOSED);
    }
    assert(allocation_count == before && live_allocations == live);
    destroy_guards();
}

/** Leaf's new edge points to a cached A specialization whose outgoing edge
 * already points back to Leaf. The immediate target A is not the current Leaf
 * ancestor; only checking all cached descendants detects the forbidden path.
 */
static void it_rechecks_cached_descendants_before_publishing_a_new_creation(void) {
    const StaticTerm *leaf = setup_families(true);
    const StaticCreationPath *b = root(target_module, family(caller_module, leaf));
    const StaticCreationPath *a = child(b), *tail = child(a);
    const StaticCreationPath *leaf_root;
    assert(static_creation_root(&guards, tail->owner->environment, &leaf_root));
    const StaticTermEnvironment *environment = leaf_root->owner->environment;
    StaticApplicationContext context;
    const AstTypeExpression *expression = field(environment->definition, "back");
    assert(static_application_context(&applications, environment, expression, &context));
    StaticApplicationInput inputs[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = leaf}};
    const StaticApplicationRecord *record;
    assert(static_application_forward(&applications, caller_constructor, inputs, 2, context, &record));
    assert(record->value == a->owner->value);
    const StaticCreationPath *output = NULL;
    StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
    assert(!static_creation_enter(&guards, leaf_root, record, &output, &outcome));
    assert(output == NULL && outcome == STATIC_CREATION_PENDING && guards.status == STATIC_GUARD_CONFLICT);
    assert(guards.creation_witness && guards.closing_edge && guards.vertex_count == 3 && guards.edge_count == 3);
    assert(guards.creation_witness->parent != NULL && diagnostic_valid(&guards.error));
    destroy_guards();
}

/** Prepare a real field omission from a symbolic generic owner. */
static const StaticCreationPath *symbolic_child(const StaticCreationPath *parent, const char *name, const StaticTerm *constructor) {
    StaticApplicationContext context;
    assert(static_application_context(&applications, parent->owner->environment,
        field(parent->owner->environment->definition, name), &context));
    const StaticApplicationRecord *record;
    assert(static_application_forward(&applications, constructor, NULL, 0, context, &record));
    const StaticCreationPath *path;
    StaticCreationOutcome outcome;
    assert(static_creation_enter(&guards, parent, record, &path, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED);
    return path;
}

/** Original symbolic keys remain distinct occurrences until independent evidence
 * closes them. A->B and B->A then share full keys and must be rechecked as a
 * genuine cycle; an external solve cannot hide this change behind dirty=false.
 */
static void it_rechecks_all_incoming_provenance_when_symbolic_keys_close(void) {
    setup_guards("@<\n...a\n...b\n@!\nanchor&\n..integer\nchild&\n.other\n",
        "@<\n...a\n...b\n@!\nback&\n.main\n");
    const StaticCreationPath *a, *b;
    assert(static_creation_root(&guards, caller_environment, &a));
    (void)symbolic_child(a, "child", target_constructor);
    const StaticTermEnvironment *b_environment;
    assert(static_term_environment(&terms, target_module, NULL, &b_environment));
    assert(static_creation_root(&guards, b_environment, &b));
    (void)symbolic_child(b, "back", caller_constructor);
    assert(guards.path_count == 4 && guards.vertex_count == 0);
    for (const StaticCreationPath *path = guards.first_path; path; path = path->next) {
        for (const StaticTermCapture *capture = path->owner->value->as.nominal.first_capture; capture; capture = capture->next) {
            assert(static_constraint_equate(&solver, capture->value, integer_type, capture->formal->site));
        }
    }
    StaticConstraintOutcome solved;
    assert(static_constraint_solve(&solver, &solved) && !solver.dirty);
    StaticGuardClosure closure = STATIC_GUARD_PENDING;
    assert(!static_guard_refresh(&guards, &closure));
    assert(closure == STATIC_GUARD_PENDING && guards.status == STATIC_GUARD_CONFLICT);
    assert(guards.vertex_count == 2 && guards.edge_count == 2 && guards.creation_witness);
    assert(a->owner->environment == caller_environment && a->owner->value != a->vertex->value);
    destroy_guards();
}

/** Execute finite concrete paths, warmed descendants and delayed key merging. */
static void run(void) {
    it_preserves_distinct_specializations_and_shared_finite_tails();
    it_rejects_a_captured_own_constructor_even_with_finite_next_captures();
    it_rechecks_cached_descendants_before_publishing_a_new_creation();
    it_rechecks_all_incoming_provenance_when_symbolic_keys_close();
}
/** Return success after counted guard and source teardown. */
int main(void) { run(); return 0; }
