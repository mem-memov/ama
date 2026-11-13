/** @file
 * @brief Check finite deep/empty/parallel dependency shapes and every graph allocation failure with counted owners.
 */
/** Own a finite graph until after observations; failure injection affects only graph records and cycle notes. */
static size_t check_graph_shape(size_t count, bool cyclic, size_t failure) {
    assert(count <= 1024 && (!cyclic || count != 0));
    setup_test();
    failure_at = failure;
    size_t identities[1024];
    GraphVertex *vertices[1024];
    DependencyGraph graph;
    dependency_graph_init(&graph, sources.allocator);
    bool built = true;
    for (size_t index = 0; index < count && built; index++) {
        vertices[index] = dependency_vertex(&graph, &identities[index], ".vertex", index, (SourceSpan){0});
        built = vertices[index] != NULL;
    }
    for (size_t index = 0; index < count && built; index++) {
        if (index + 1 == count && !cyclic) { continue; }
        size_t target = (index + 1) % count;
        /* Parallel edges preserve the prerequisite; neither ordering nor cycle extraction may count them as new vertices. */
        built = dependency_edge(&graph, vertices[index], vertices[target], (SourceSpan){0});
        if (built) { built = dependency_edge(&graph, vertices[index], vertices[target], (SourceSpan){0}); }
    }
    GraphStatus status = dependency_order(&graph);
    if (failure != 0) {
        assert(status == GRAPH_OUT_OF_MEMORY && !graph.valid && diagnostic_valid(&graph.error));
    } else if (cyclic) {
        assert(status == GRAPH_CYCLE && graph.error.related_count == count && !graph.valid);
    } else {
        assert(status == GRAPH_OK && graph.valid);
        size_t remaining = count;
        for (GraphVertex *vertex = graph.order; vertex != NULL; vertex = vertex->order_next) {
            assert(remaining != 0 && vertex == vertices[remaining - 1]);
            remaining--;
        }
        assert(remaining == 0);
    }
    size_t allocations = allocation_count;
    dependency_graph_destroy(&graph);
    destroy_test();
    return allocations;
}
/** Empty and deep graphs have no recursive depth threshold; every allocation in success/cycle shapes fails cleanly. */
static void it_handles_empty_deep_parallel_and_allocation_failed_graphs(void) {
    (void)check_graph_shape(0, false, 0);
    (void)check_graph_shape(1024, false, 0);
    (void)check_graph_shape(1024, true, 0);
    for (size_t cyclic = 0; cyclic < 2; cyclic++) {
        size_t allocations = check_graph_shape(4, cyclic != 0, 0);
        for (size_t failure = 1; failure <= allocations; failure++) {
            (void)check_graph_shape(4, cyclic != 0, failure);
        }
    }
}
