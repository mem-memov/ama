/** @file
 * @brief Verify independent runtime Type holes, strict source membership and solver diagnostics.
 */
#include "../../common.c"
#include "semantic/static/terms/store.h"
#include "semantic/static/constraints/solver.h"
#include <stdint.h>

/** Term owners borrow only immutable inventory/source/environment identities. */
static StaticTermStore store;
static const StaticModule *module;
static const StaticTermEnvironment *environment;
static const AstNode *assignment, *call, *traversal, *indexed;

/** Parser-valid runtime operands need no guessed runtime binding or concrete Type. */
static void setup(void) {
    setup_static("@<\ninput\n.type\n@!\nfirst\n earlier\nworker !\n > left\n > right\n"
        "values[item&]\n{\n}\nvalues[position]\n earlier\n", "@!\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    module = static_module_find(&inventory, project.entry);
    static_term_store_init(&store, &inventory, sources.allocator);
    assert(static_term_environment(&store, module, NULL, &environment));
    assignment = project.entry->syntax.body.first;
    call = assignment->next;
    traversal = call->next;
    indexed = traversal->next;
    assert(layout_text_is(assignment->as.assignment.receiver->name, "first"));
    assert(static_name_site(project.entry, "first").start == assignment->as.assignment.receiver->name.start);
}

/** The term store expires before its borrowed project/inventory owners. */
static void teardown(void) { static_term_store_destroy(&store); destroy_static(); }

/** Every original receiver/header and lexical environment identifies a distinct
 * Type variable. Equal retries allocate nothing, including with refusal armed.
 */
static void it_interns_distinct_runtime_receiver_and_foreach_type_holes_per_environment(void) {
    setup();
    const StaticTerm *first, *left, *right, *local, *again, *formal;
    assert(static_term_runtime_type(&store, environment, assignment, assignment->as.assignment.receiver, &first));
    assert(static_term_runtime_type(&store, environment, call, call->as.call.first->receiver, &left));
    assert(static_term_runtime_type(&store, environment, call, call->as.call.first->next->receiver, &right));
    assert(static_term_runtime_type(&store, environment, traversal, NULL, &local));
    assert(first != left && first != right && first != local && left != right && left != local && right != local);
    assert(first->kind == STATIC_TERM_VARIABLE && first->category == STATIC_CATEGORY_TYPE && first->signature == NULL);
    assert(first->as.variable.role == STATIC_VARIABLE_RUNTIME && first->as.variable.symbol == NULL);
    assert(first->as.variable.runtime_access == assignment->as.assignment.receiver && local->as.variable.declaration == traversal);
    assert(static_term_symbol(&store, environment, module->first, &formal) && formal != first);
    size_t before = allocation_count;
    failure_at = before + 1;
    assert(static_term_runtime_type(&store, environment, assignment, assignment->as.assignment.receiver, &again));
    assert(again == first && allocation_count == before);
    failure_at = 0;
    const StaticModule *other = static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/other"));
    const StaticTerm *other_type, *owner;
    const StaticTermEnvironment *applied;
    assert(static_term_nominal(&store, other, NULL, 0, (SourceSpan){0}, &other_type));
    StaticTermCapture capture = {.formal = module->first, .value = other_type};
    assert(static_term_nominal(&store, module, &capture, 1, (SourceSpan){0}, &owner));
    assert(static_term_environment(&store, module, owner, &applied));
    assert(static_term_runtime_type(&store, applied, assignment, assignment->as.assignment.receiver, &again) && again != first);
    teardown();
}

/** Copied/foreign owners, copied nodes/accesses and indexed receivers cannot
 * impersonate an original source-introduced simple field or foreach binding.
 */
static void it_refuses_invalid_runtime_hole_membership_and_indexed_introductions(void) {
    for (size_t mode = 0; mode < 7; mode++) {
        setup();
        StaticTermEnvironment copied_environment = *environment;
        AstNode copied_node = *assignment;
        AstAccess copied_access = *assignment->as.assignment.receiver;
        const StaticTerm *value = NULL;
        const AstNode *node = mode == 1 ? &copied_node : mode == 4 ? indexed : assignment;
        const AstAccess *access = mode == 2 ? &copied_access : mode == 3 ? call->as.call.first->receiver :
            mode == 4 ? indexed->as.assignment.receiver : mode == 5 ? NULL : assignment->as.assignment.receiver;
        assert(!static_term_runtime_type(&store, mode == 0 ? &copied_environment : environment,
            node, access, mode == 6 ? NULL : &value));
        assert(value == NULL && store.status == STATIC_TERM_INVALID && store.term_count == 0);
        teardown();
    }
}

/** Internal runtime variables have original source sites without a fabricated
 * static formal or producer expression. Unresolved diagnostics must use that site.
 */
static void it_reports_unresolved_runtime_types_at_the_original_receiver_without_a_static_symbol(void) {
    setup();
    const StaticTerm *value;
    assert(static_term_runtime_type(&store, environment, assignment, assignment->as.assignment.receiver, &value));
    StaticConstraintSolver solver;
    static_constraint_init(&solver, &store, sources.allocator);
    assert(static_constraint_require(&solver, value, value->site));
    assert(!static_constraint_finish(&solver) && solver.status == STATIC_CONSTRAINT_UNRESOLVED);
    assert(diagnostic_valid(&solver.error) && solver.error.related_count > 0);
    for (size_t index = 0; index < solver.error.related_count; index++) {
        assert(solver.error.related[index].location.span.source == value->site.source);
        assert(solver.error.related[index].location.span.start == value->site.start);
    }
    static_constraint_destroy(&solver);
    teardown();
}

/** Refuse every actual hole allocation and explicit counter overflow before
 * publication; public outputs and existing store identities remain unchanged.
 */
static void it_unwinds_every_runtime_hole_allocation_and_counter_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup();
        const StaticTerm *value = NULL;
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        bool success = static_term_runtime_type(&store, environment, assignment, assignment->as.assignment.receiver, &value);
        if (!cut) { assert(success && value != NULL); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && value == NULL && store.status == STATIC_TERM_OUT_OF_MEMORY && store.term_count == 0); }
        failure_at = 0;
        teardown();
    }
    setup();
    store.term_count = SIZE_MAX;
    const StaticTerm *value = NULL;
    size_t before = allocation_count;
    assert(!static_term_runtime_type(&store, environment, traversal, NULL, &value));
    assert(value == NULL && store.status == STATIC_TERM_OUT_OF_MEMORY && allocation_count == before && store.first == NULL);
    teardown();
}

/** Run exact identity, source ownership, diagnostic and resource partitions. */
static void run(void) {
    it_interns_distinct_runtime_receiver_and_foreach_type_holes_per_environment();
    it_refuses_invalid_runtime_hole_membership_and_indexed_introductions();
    it_reports_unresolved_runtime_types_at_the_original_receiver_without_a_static_symbol();
    it_unwinds_every_runtime_hole_allocation_and_counter_refusal();
}
/** Return only after all assertions and counted teardown complete. */
int main(void) { run(); return 0; }
