/**
 * @file
 * @brief Check immutable shared construction plans and dependency ordering.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../typed-common.c"
#include "support/construction.c"
#include "support/failures.c"

/**
 * Write the explicit immutable-value dependency fixture used to inspect construction order and
 * shared reads.
 */
static SemanticStatus prepare_constant_plan(void) {
    prepare_types();
    file("generated/program/worker", "@=\nfirst\n.value\n |+\nsecond\n.value\n |-\n");
    SemanticStatus status = resolve_text("@=\nfirst\n.value\n .worker.second\nsecond\n.value\n first\n"
        "@!\nreceiver\n.value\n.worker\n first >> receiver\n");
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    return semantic_constants(&semantic);
}

/**
 * Assert that the component plans shared initialization once and cleanup in exact reverse order.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_plans_shared_initialization_once_and_cleanup_in_exact_reverse_order(void) {
    SemanticStatus status = prepare_constant_plan();
    assert(status == SEMANTIC_OK && semantic.constants_valid && semantic.constant_count == 4);
    SemanticConstantPlan *entry = semantic.constant_first;
    for (size_t ordinal = 0; ordinal < 4; ordinal++) {
        assert(entry != NULL && entry->ordinal == ordinal && entry->symbol->shared == entry);
        assert(entry->symbol->role == SYMBOL_CONSTANT);
        if (ordinal < 2) {
            assert(strcmp(entry->symbol->owner->module->source->logical_path, ".worker") == 0);
        } else {
            assert(entry->symbol->owner == semantic.first);
        }
        if (entry->next != NULL) {
            assert(entry->next->previous == entry);
        }
        entry = entry->next;
    }
    assert(entry == NULL && semantic.constant_first->previous == NULL);
    entry = semantic.constant_last;
    for (size_t completed = 4; completed > 0; completed--) {
        assert(entry->ordinal == completed - 1);
        entry = entry->previous;
    }
    assert(entry == NULL);
    SemanticStatement *read = semantic.first->last_statement;
    assert(read->syntax->as.call.mode == AST_CONSTANT_READ && read->access == NULL);
    assert(read->bindings->field->shared == semantic.constant_first);
    assert(!read->namespace_type->executable);
    assert(layout_text_is(semantic.constant_first->initializers->source->syntax->payload, "+"));
    destroy_semantic();
}

/**
 * Assert that the component uses path order for ready constants and excludes unused namespaces.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_uses_path_order_for_ready_constants_and_excludes_unused_namespaces(void) {
    prepare_types();
    file("generated/program/worker", "@=\nfirst\n.value\n |worker\n");
    file("generated/program/unused", "@=\nvalue\n.value\n |unused\n");
    SemanticStatus status = resolve_text("@=\nfirst\n.value\n |main\n@!\nreceiver\n.value\n.worker\n >> receiver\n");
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    status = semantic_constants(&semantic);
    assert(status == SEMANTIC_OK && semantic.constant_count == 2);
    assert(semantic.constant_first->symbol->owner == semantic.first);
    assert(semantic_find_type(&semantic, ".unused") == NULL);
    destroy_semantic();
}

/**
 * Assert that the component releases a partially built shared storage plan. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_releases_a_partially_built_shared_storage_plan(void) {
    SemanticStatus status = prepare_constant_plan();
    assert(status == SEMANTIC_OK);
    size_t count = semantic.constant_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= count; failure++) {
        prepare_types();
        file("generated/program/worker", "@=\nfirst\n.value\n |+\nsecond\n.value\n |-\n");
        status = resolve_text("@=\nfirst\n.value\n .worker.second\nsecond\n.value\n first\n@!\n");
        assert(status == SEMANTIC_OK);
        status = semantic_relations(&semantic);
        assert(status == SEMANTIC_OK);
        failure_at = allocation_count + failure;
        status = semantic_constants(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.constants_valid);
        destroy_semantic();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_unwinds_every_nested_shared_graph_cycle_and_plan_allocation();
    it_rejects_invalid_constant_members_forms_and_forward_reads();
    it_rejects_self_handles_inside_fresh_shared_descendants();
    it_constructs_inferred_fields_from_defaults_without_reading_their_body_sources();
    it_preserves_each_indexed_shared_read_without_duplicate_root_edges();
    it_follows_fresh_descendants_but_does_not_reconstruct_cloned_values();
    it_plans_shared_initialization_once_and_cleanup_in_exact_reverse_order();
    it_uses_path_order_for_ready_constants_and_excludes_unused_namespaces();
    it_releases_a_partially_built_shared_storage_plan();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
