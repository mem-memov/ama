/**
 * @file
 * @brief Check eager-layout, shared-read and copy-capability graph relations independently.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../typed-common.c"
#include "semantic/internal.h"
#include "support/capability.c"
#include "support/eager.c"
#include "support/order.c"
#include "support/failures.c"

/**
 * Resolve the supplied declarations and compare actual eager/copy relation results, including
 * complete error diagnostics on rejected graphs.
 */
static void expect_relations(const char *text, bool valid) {
    SemanticStatus status = resolve_text(text);
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    if ((status == SEMANTIC_OK) != valid) {
        fprintf(stderr, "Unexpected relation check: %s\n%s\n", semantic.error.message, text);
    }
    assert((status == SEMANTIC_OK) == valid);
    if (!valid) {
        assert(diagnostic_valid(&semantic.error));
    }
    destroy_semantic();
}

/**
 * Assert that the component distinguishes loading eager construction and clone cycles. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_distinguishes_loading_eager_construction_and_clone_cycles(void) {
    prepare_types();
    expect_relations("@!\nchildren[]\n/test/fixture/v1.0.0/src/main\nparent&\n/test/fixture/v1.0.0/src/main\n", true);
    expect_relations("@!\nchild\n/test/fixture/v1.0.0/src/main\n", false);
    native_fixture("main", "+ initialize\n- destroy\n| literal\n= clone\n");
    expect_relations("@<\nchildren[]\n/test/fixture/v1.0.0/src/main\n |element\n@C\n", false);
}

/**
 * Assert that the component accepts recursive empty lists and propagates noncopyable owned
 * edges. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_accepts_recursive_empty_lists_and_propagates_noncopyable_owned_edges(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\nchildren[]\n/test/fixture/v1.0.0/src/worker\n");
    SemanticStatus status = resolve_text("@!\nworker\n/test/fixture/v1.0.0/src/worker\ncopy\n/test/fixture/v1.0.0/src/worker\n worker\n");
    assert(status == SEMANTIC_OK && semantic.last->cloneable);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    destroy_semantic();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\nchildren[]\n/test/fixture/v1.0.0/src/worker\nresource\n/test/fixture/v1.0.0/src/resource\n");
    status = resolve_text("@!\nworker\n/test/fixture/v1.0.0/src/worker\ncopy\n/test/fixture/v1.0.0/src/worker\n worker\n");
    assert(status == SEMANTIC_INVALID);
    destroy_semantic();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\nchildren[]\n/test/fixture/v1.0.0/src/worker\nresource&\n/test/fixture/v1.0.0/src/resource\n");
    expect_relations("@!\nworker\n/test/fixture/v1.0.0/src/worker\ncopy\n/test/fixture/v1.0.0/src/worker\n worker\n", true);
}

/**
 * Assert that the component orders constants by declarations and actual value reads. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_orders_constants_by_declarations_and_actual_value_reads(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@=\nvalue\n/test/fixture/v1.0.0/src/value\n |ready\n");
    SemanticStatus status = resolve_text("@=\nfirst\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/value\nsecond\n/test/fixture/v1.0.0/src/value\n first\n@!\n");
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    GraphVertex *order = semantic.constants.order;
    const SemanticSymbol *constant = order->value;
    assert(strcmp(constant->owner->module->source->logical_path, "/test/fixture/v1.0.0/src/worker") == 0);
    constant = order->order_next->value;
    assert(constant->role_ordinal == 0 && constant->owner == semantic_entry());
    assert(order->order_next->order_next->order_next == NULL);
    destroy_semantic();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@=\nvalue\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/main/value\n");
    expect_relations("@=\nvalue\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/value\n@!\n", false);
}

/**
 * Assert that the component follows owned field construction reads without treating shared types
 * as cycles. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_follows_owned_field_construction_reads_without_treating_shared_types_as_cycles(void) {
    prepare_types();
    expect_relations("@=\nself\n/test/fixture/v1.0.0/src/main\n@!\n", true);
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@=\nvalue\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/main/second\n@!\nfield\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/value\n");
    expect_relations("@=\nfirst\n/test/fixture/v1.0.0/src/worker\nsecond\n/test/fixture/v1.0.0/src/value\n |ready\n@!\n", false);
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\nvalue\n/test/fixture/v1.0.0/src/value\nreference&\n/test/fixture/v1.0.0/src/value\n & value\n");
    expect_relations("@=\nconstant\n/test/fixture/v1.0.0/src/worker\n@!\n", false);
}

/** Map a diagnostic witness to one exact vertex in the independent finite
 * graph. Complete canonical identities, not character positions or basenames,
 * identify the expected vertex after source namespace changes.
 */
static size_t model_vertex_index(const char *name, const char *const *names, size_t count) {
    for (size_t index = 0; index < count; index++) {
        if (strcmp(name, names[index]) == 0) { return index; }
    }
    assert(false);
    return count;
}

/**
 * Assert that the component orders every three vertex graph or reports a real cycle. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_orders_every_three_vertex_graph_or_reports_a_real_cycle(void) {
    int identities[3] = {0};
    for (unsigned bits = 0; bits < 512; bits++) {
        setup_test();
        DependencyGraph graph;
        dependency_graph_init(&graph, sources.allocator);
        GraphVertex *vertices[3];
        const char *names[] = {"/test/fixture/v1.0.0/src/a", "/test/fixture/v1.0.0/src/b", "/test/fixture/v1.0.0/src/c"};
        for (size_t index = 0; index < 3; index++) {
            vertices[index] = dependency_vertex(&graph, &identities[index], names[index], 0, (SourceSpan){0});
            assert(vertices[index] != NULL);
        }
        bool reach[3][3] = {{false}};
        for (size_t from = 0; from < 3; from++) {
            for (size_t to = 0; to < 3; to++) {
                reach[from][to] = (bits & (1u << (from * 3 + to))) != 0;
                if (reach[from][to]) {
                    bool added = dependency_edge(&graph, vertices[from], vertices[to], (SourceSpan){0});
                    assert(added);
                }
            }
        }
        for (size_t via = 0; via < 3; via++) {
            for (size_t from = 0; from < 3; from++) {
                for (size_t to = 0; to < 3; to++) {
                    reach[from][to] |= reach[from][via] && reach[via][to];
                }
            }
        }
        bool cyclic = reach[0][0] || reach[1][1] || reach[2][2];
        GraphStatus status = dependency_order(&graph);
        assert(status == (cyclic ? GRAPH_CYCLE : GRAPH_OK));
        if (cyclic) {
            assert(diagnostic_valid(&graph.error) && graph.error.related_count > 0);
            size_t first = model_vertex_index(graph.error.subject, names, 3);
            size_t from = first;
            for (size_t index = 0; index < graph.error.related_count; index++) {
                size_t to = model_vertex_index(graph.error.related[index].name, names, 3);
                assert(from < 3 && to < 3 && (bits & (1u << (from * 3 + to))) != 0);
                from = to;
            }
            assert(from == first);
        } else {
            bool seen[3] = {false};
            size_t count = 0;
            for (GraphVertex *vertex = graph.order; vertex != NULL; vertex = vertex->order_next) {
                size_t index = (size_t)((const int *)vertex->value - identities);
                assert(index < 3 && !seen[index]);
                for (GraphEdge *edge = vertex->first; edge != NULL; edge = edge->next) {
                    size_t dependency = (size_t)((const int *)edge->target->value - identities);
                    assert(seen[dependency]);
                }
                seen[index] = true;
                count++;
            }
            assert(count == 3);
        }
        dependency_graph_destroy(&graph);
        destroy_test();
    }
}

/**
 * Write the explicitly enumerated ownership/reference graph as Ama declarations for exact
 * relation checks; no graph is inferred from runtime values.
 */
static void graph_interface(char *text, size_t capacity, unsigned edges, size_t vertex) {
    const char *names[] = {"main", "left", "right"};
    int written = snprintf(text, capacity, "@<\nleft&\n/test/fixture/v1.0.0/src/left\nright&\n/test/fixture/v1.0.0/src/right\n");
    assert(written > 0 && (size_t)written < capacity);
    size_t used = (size_t)written;
    for (size_t target = 0; target < 3; target++) {
        if ((edges & (1u << (vertex * 3 + target))) != 0) {
            written = snprintf(text + used, capacity - used, "field%zu[]\n/test/fixture/v1.0.0/src/%s\n", target, names[target]);
            assert(written > 0 && (size_t)written < capacity - used);
            used += (size_t)written;
        }
    }
    written = snprintf(text + used, capacity - used, "@C\n");
    assert(written > 0 && (size_t)written < capacity - used);
}

/**
 * Assert that the component matches clone capability to reachability of a noncopyable native
 * type. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_matches_clone_capability_to_reachability_of_a_noncopyable_native_type(void) {
    prepare_types();
    const char *cloneable = "+ initialize\n- destroy\n= clone\n";
    native_fixture("main", cloneable);
    native_fixture("left", cloneable);
    native_fixture("right", "+ initialize\n- destroy\n");
    for (unsigned bits = 0; bits < 512; bits++) {
        char main_text[256];
        char other_text[256];
        graph_interface(main_text, sizeof(main_text), bits, 0);
        graph_interface(other_text, sizeof(other_text), bits, 1);
        file("generated/program/packages/test/fixture/v1.0.0/src/left", other_text);
        graph_interface(other_text, sizeof(other_text), bits, 2);
        file("generated/program/packages/test/fixture/v1.0.0/src/right", other_text);
        SemanticStatus status = resolve_text(main_text);
        assert(status == SEMANTIC_OK);
        bool reaches[3][3] = {{false}};
        for (size_t from = 0; from < 3; from++) {
            for (size_t to = 0; to < 3; to++) {
                reaches[from][to] = (bits & (1u << (from * 3 + to))) != 0;
            }
        }
        for (size_t via = 0; via < 3; via++) {
            for (size_t from = 0; from < 3; from++) {
                for (size_t to = 0; to < 3; to++) {
                    reaches[from][to] |= reaches[from][via] && reaches[via][to];
                }
            }
        }
        const char *paths[] = {"/test/fixture/v1.0.0/src/main", "/test/fixture/v1.0.0/src/left", "/test/fixture/v1.0.0/src/right"};
        for (size_t vertex = 0; vertex < 3; vertex++) {
            SemanticType *type = semantic_find_type(&semantic, paths[vertex]);
            assert(type != NULL && type->cloneable == (vertex != 2 && !reaches[vertex][2]));
        }
        destroy_semantic();
    }
}

/**
 * Assert that the component cleans successful and cyclic relation plans after every allocation
 * failure. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_cleans_successful_and_cyclic_relation_plans_after_every_allocation_failure(void) {
    prepare_types();
    const char *texts[] = {"@=\nfirst\n/test/fixture/v1.0.0/src/value\n |one\nsecond\n/test/fixture/v1.0.0/src/value\n first\n@!\n", "@!\nchild\n/test/fixture/v1.0.0/src/main\n"};
    for (size_t test = 0; test < 2; test++) {
        SemanticStatus status = resolve_text(texts[test]);
        assert(status == SEMANTIC_OK);
        size_t before = allocation_count;
        status = semantic_relations(&semantic);
        assert(status == (test == 0 ? SEMANTIC_OK : SEMANTIC_INVALID));
        size_t count = allocation_count - before;
        destroy_semantic();
        for (size_t failure = 1; failure <= count; failure++) {
            status = resolve_text(texts[test]);
            assert(status == SEMANTIC_OK);
            failure_at = allocation_count + failure;
            status = semantic_relations(&semantic);
            assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.relations_valid);
            assert(diagnostic_valid(&semantic.error));
            destroy_semantic();
        }
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_handles_empty_deep_parallel_and_allocation_failed_graphs();
    it_orders_graphs_independently_of_vertex_and_edge_admission_order();
    it_matches_every_three_type_storage_graph_and_native_seed_set();
    it_classifies_every_eager_field_role_form_and_initializer_state();
    it_keeps_inferred_default_storage_in_the_correct_relations();
    it_distinguishes_loading_eager_construction_and_clone_cycles();
    it_accepts_recursive_empty_lists_and_propagates_noncopyable_owned_edges();
    it_orders_constants_by_declarations_and_actual_value_reads();
    it_follows_owned_field_construction_reads_without_treating_shared_types_as_cycles();
    it_orders_every_three_vertex_graph_or_reports_a_real_cycle();
    it_cleans_successful_and_cyclic_relation_plans_after_every_allocation_failure();
    it_matches_clone_capability_to_reachability_of_a_noncopyable_native_type();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
