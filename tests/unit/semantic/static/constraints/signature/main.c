/** @file
 * @brief Verify actual constructor comparison scratch, solver caching and failure ownership.
 */
#include "../../common.c"
#include "semantic/static/signature/signature.h"
#include "semantic/static/constraints/internal.h"

/** Reordered ordinary Type formals supply actual constructor metadata. No
 * explicit foreign-signature declaration or test-only source conversion exists.
 */
static void setup_signature_sources(const StaticSignature **left, const StaticSignature **right) {
    setup_static("@<\nkey\n.key\nvalue\n.value\n@!\n",
        "@<\nvalue\n.value\nkey\n.key\n@!\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(module->first->site.start == static_name_site(project.entry, ".key").start);
    *left = module->constructor_signature;
    *right = static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/other"))->constructor_signature;
    assert(*left != NULL && *right != NULL && (*left)->count == 2 && (*right)->count == 2);
}

/** Actual reordered signatures compare exactly while the accepted source
 * inventory retains its arena, status, definitions and immutable metadata.
 */
static void it_compares_actual_constructor_signatures_in_independent_read_only_scratch(void) {
    const StaticSignature *left, *right;
    setup_signature_sources(&left, &right);
    SourceAllocation *original = inventory.arena.first;
    SourceArena scratch;
    source_arena_init(&scratch, sources.allocator);
    StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
    assert(static_signature_compare_arena(&scratch, left, right, &result));
    assert(result == STATIC_SIGNATURE_EQUAL && inventory.arena.first == original);
    assert(inventory.valid && inventory.status == STATIC_OK);
    source_arena_destroy(&scratch);
    destroy_static();
}

/** Every observed scratch refusal preserves the output and accepted inventory;
 * partially allocated scratch is separately and completely destructible.
 */
static void it_preserves_output_and_source_inventory_on_each_signature_scratch_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        const StaticSignature *left, *right;
        setup_signature_sources(&left, &right);
        SourceAllocation *original = inventory.arena.first;
        SourceArena scratch;
        source_arena_init(&scratch, sources.allocator);
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
        bool success = static_signature_compare_arena(&scratch, left, right, &result);
        if (failure == 0) { assert(success && result == STATIC_SIGNATURE_EQUAL); total = allocation_count - before; assert(total != 0); }
        else { assert(!success && result == STATIC_SIGNATURE_DIFFERENT); }
        assert(inventory.arena.first == original && inventory.valid && inventory.status == STATIC_OK);
        failure_at = 0;
        source_arena_destroy(&scratch);
        destroy_static();
    }
}

/** The real constraint consumer owns the immutable cached outcome and releases
 * temporary scratch before publication; reversed lookup performs no allocation.
 */
static void it_caches_actual_signature_outcomes_without_mutating_source_metadata(void) {
    const StaticSignature *left, *right;
    setup_signature_sources(&left, &right);
    SourceAllocation *original = inventory.arena.first;
    StaticTermStore terms;
    static_term_store_init(&terms, &inventory, sources.allocator);
    StaticConstraintSolver solver;
    static_constraint_init(&solver, &terms, sources.allocator);
    StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
    SourceSpan site = left->first->name;
    assert(static_constraint_signature(&solver, left, right, site, &result));
    assert(result == STATIC_SIGNATURE_EQUAL && solver.signature_count == 1);
    const StaticConstraintSignature *cached = solver.first_signature;
    assert(cached != NULL && cached->next == NULL);
    size_t before = allocation_count;
    result = STATIC_SIGNATURE_DIFFERENT;
    assert(static_constraint_signature(&solver, right, left, site, &result));
    assert(result == STATIC_SIGNATURE_EQUAL && allocation_count == before);
    assert(solver.first_signature == cached && solver.signature_count == 1);
    assert(inventory.arena.first == original && inventory.valid && inventory.status == STATIC_OK);
    static_constraint_destroy(&solver);
    static_term_store_destroy(&terms);
    destroy_static();
}

/** Scratch and cache-entry refusals are distinguished from semantic mismatch:
 * no partial cache entry or caller result is published on either path.
 */
static void it_unwinds_every_actual_signature_cache_preparation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        const StaticSignature *left, *right;
        setup_signature_sources(&left, &right);
        SourceAllocation *original = inventory.arena.first;
        StaticTermStore terms;
        static_term_store_init(&terms, &inventory, sources.allocator);
        StaticConstraintSolver solver;
        static_constraint_init(&solver, &terms, sources.allocator);
        StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool success = static_constraint_signature(&solver, left, right, left->first->name, &result);
        if (failure == 0) { assert(success && result == STATIC_SIGNATURE_EQUAL); total = allocation_count - before; assert(total != 0); }
        else {
            assert(!success && solver.status == STATIC_CONSTRAINT_OUT_OF_MEMORY);
            assert(result == STATIC_SIGNATURE_DIFFERENT && solver.first_signature == NULL && solver.signature_count == 0);
            assert(diagnostic_valid(&solver.error));
        }
        assert(inventory.arena.first == original && inventory.valid && inventory.status == STATIC_OK);
        failure_at = 0;
        static_constraint_destroy(&solver);
        static_term_store_destroy(&terms);
        destroy_static();
    }
}

/** Execute primitive scratch and actual consumer ownership partitions. */
static void run(void) {
    it_compares_actual_constructor_signatures_in_independent_read_only_scratch();
    it_preserves_output_and_source_inventory_on_each_signature_scratch_refusal();
    it_caches_actual_signature_outcomes_without_mutating_source_metadata();
    it_unwinds_every_actual_signature_cache_preparation_refusal();
}
/** Return success only after assertions and complete counted teardown. */
int main(void) { run(); return 0; }
