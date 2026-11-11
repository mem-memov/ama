#include "../typed-common.c"

static void expect_relations(const char *text, bool valid) {
    SemanticStatus status = resolve_text(text, (SemanticLibraries){0});
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

static void it_distinguishes_loading_eager_construction_and_clone_cycles(void) {
    prepare_types();
    expect_relations("@!\nchildren[]\n.main\nparent&\n.main\n", true);
    expect_relations("@!\nchild\n.main\n", false);
    native_fixture("main", "+ initialize\n- destroy\n| literal\n= clone\n");
    expect_relations("@<\nchildren[]\n.main\n |element\n@C\n", false);
}

static void it_accepts_recursive_empty_lists_and_propagates_noncopyable_owned_edges(void) {
    prepare_types();
    file("generated/program/worker", "@!\nchildren[]\n.worker\n");
    SemanticStatus status = resolve_text("@!\nworker\n.worker\ncopy\n.worker\n worker\n", (SemanticLibraries){0});
    assert(status == SEMANTIC_OK && semantic.last->cloneable);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    destroy_semantic();
    file("generated/program/worker", "@!\nchildren[]\n.worker\nresource\n.resource\n");
    status = resolve_text("@!\nworker\n.worker\ncopy\n.worker\n worker\n", (SemanticLibraries){0});
    assert(status == SEMANTIC_INVALID);
    destroy_semantic();
    file("generated/program/worker", "@!\nchildren[]\n.worker\nresource&\n.resource\n");
    expect_relations("@!\nworker\n.worker\ncopy\n.worker\n worker\n", true);
}

static void it_orders_constants_by_declarations_and_actual_value_reads(void) {
    prepare_types();
    file("generated/program/worker", "@=\nvalue\n.value\n |ready\n");
    SemanticStatus status = resolve_text("@=\nfirst\n.value\n .worker.value\nsecond\n.value\n first\n@!\n", (SemanticLibraries){0});
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    GraphVertex *order = semantic.constants.order;
    const SemanticSymbol *constant = order->value;
    assert(strcmp(constant->owner->module->source->logical_path, ".worker") == 0);
    constant = order->order_next->value;
    assert(constant->role_ordinal == 0 && constant->owner == semantic.first);
    assert(order->order_next->order_next->order_next == NULL);
    destroy_semantic();
    file("generated/program/worker", "@=\nvalue\n.value\n .main.value\n");
    expect_relations("@=\nvalue\n.value\n .worker.value\n@!\n", false);
}

static void it_follows_owned_field_construction_reads_without_treating_shared_types_as_cycles(void) {
    prepare_types();
    expect_relations("@=\nself\n.main\n@!\n", true);
    file("generated/program/worker", "@=\nvalue\n.value\n .main.second\n@!\nfield\n.value\n .worker.value\n");
    expect_relations("@=\nfirst\n.worker\nsecond\n.value\n |ready\n@!\n", false);
    file("generated/program/worker", "@!\nvalue\n.value\nreference&\n.value\n &value\n");
    expect_relations("@=\nconstant\n.worker\n@!\n", false);
}

static void it_orders_every_three_vertex_graph_or_reports_a_real_cycle(void) {
    int identities[3] = {0};
    for (unsigned bits = 0; bits < 512; bits++) {
        setup_test();
        DependencyGraph graph;
        dependency_graph_init(&graph, sources.allocator);
        GraphVertex *vertices[3];
        const char *names[] = {".a", ".b", ".c"};
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
            size_t first = (size_t)(graph.error.subject[1] - 'a');
            size_t from = first;
            for (size_t index = 0; index < graph.error.related_count; index++) {
                size_t to = (size_t)(graph.error.related[index].name[1] - 'a');
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

static void graph_interface(char *text, size_t capacity, unsigned edges, size_t vertex) {
    const char *names[] = {"main", "left", "right"};
    int written = snprintf(text, capacity, "@<\nleft&\n.left\nright&\n.right\n");
    assert(written > 0 && (size_t)written < capacity);
    size_t used = (size_t)written;
    for (size_t target = 0; target < 3; target++) {
        if ((edges & (1u << (vertex * 3 + target))) != 0) {
            written = snprintf(text + used, capacity - used, "field%zu[]\n.%s\n", target, names[target]);
            assert(written > 0 && (size_t)written < capacity - used);
            used += (size_t)written;
        }
    }
    written = snprintf(text + used, capacity - used, "@C\n");
    assert(written > 0 && (size_t)written < capacity - used);
}

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
        file("generated/program/left", other_text);
        graph_interface(other_text, sizeof(other_text), bits, 2);
        file("generated/program/right", other_text);
        SemanticStatus status = resolve_text(main_text, (SemanticLibraries){0});
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
        const char *paths[] = {".main", ".left", ".right"};
        for (size_t vertex = 0; vertex < 3; vertex++) {
            SemanticType *type = semantic_find_type(&semantic, paths[vertex]);
            assert(type != NULL && type->cloneable == (vertex != 2 && !reaches[vertex][2]));
        }
        destroy_semantic();
    }
}

static void it_cleans_successful_and_cyclic_relation_plans_after_every_allocation_failure(void) {
    prepare_types();
    const char *texts[] = {"@=\nfirst\n.value\n |one\nsecond\n.value\n first\n@!\n", "@!\nchild\n.main\n"};
    for (size_t test = 0; test < 2; test++) {
        SemanticStatus status = resolve_text(texts[test], (SemanticLibraries){0});
        assert(status == SEMANTIC_OK);
        size_t before = allocation_count;
        status = semantic_relations(&semantic);
        assert(status == (test == 0 ? SEMANTIC_OK : SEMANTIC_INVALID));
        size_t count = allocation_count - before;
        destroy_semantic();
        for (size_t failure = 1; failure <= count; failure++) {
            status = resolve_text(texts[test], (SemanticLibraries){0});
            assert(status == SEMANTIC_OK);
            failure_at = allocation_count + failure;
            status = semantic_relations(&semantic);
            assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.relations_valid);
            assert(diagnostic_valid(&semantic.error));
            destroy_semantic();
        }
    }
}

void run(void) {
    it_distinguishes_loading_eager_construction_and_clone_cycles();
    it_accepts_recursive_empty_lists_and_propagates_noncopyable_owned_edges();
    it_orders_constants_by_declarations_and_actual_value_reads();
    it_follows_owned_field_construction_reads_without_treating_shared_types_as_cycles();
    it_orders_every_three_vertex_graph_or_reports_a_real_cycle();
    it_cleans_successful_and_cyclic_relation_plans_after_every_allocation_failure();
    it_matches_clone_capability_to_reachability_of_a_noncopyable_native_type();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
