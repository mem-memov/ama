/** @file
 * @brief Compare all small graph orders with exhaustive topological permutations and verify concrete original cycle sites.
 */
/** Enumerate complete candidate orders independently from the production ready-vertex algorithm. */
static bool before_in_order(unsigned bits, const size_t order[3]) {
    size_t positions[3];
    for (size_t index = 0; index < 3; index++) { positions[order[index]] = index; }
    for (size_t owner = 0; owner < 3; owner++) {
        for (size_t target = 0; target < 3; target++) {
            if ((bits & (1u << (owner * 3 + target))) && positions[target] >= positions[owner]) { return false; }
        }
    }
    return true;
}
/** Exact full-key comparison of complete candidate sequences, including equal paths with different ordinals. */
static bool order_precedes(const size_t left[3], const size_t right[3], const char *const names[3], const size_t ordinals[3]) {
    for (size_t index = 0; index < 3; index++) {
        int compared = strcmp(names[left[index]], names[right[index]]);
        if (compared != 0) { return compared < 0; }
        if (ordinals[left[index]] != ordinals[right[index]]) { return ordinals[left[index]] < ordinals[right[index]]; }
    }
    return false;
}
/** Every insertion order yields the least valid complete permutation or an actual closed source-mapped cycle. */
static void it_orders_graphs_independently_of_vertex_and_edge_admission_order(void) {
    const size_t permutations[6][3] = {{0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}};
    const char *keys[2][3] = {{".z", ".a", "..a"}, {".a", ".a", "..a"}};
    const size_t ordinals[2][3] = {{0, 0, 0}, {0, 1, 0}};
    int identities[3] = {0};
    for (size_t keyset = 0; keyset < 2; keyset++) {
        for (unsigned bits = 0; bits < 512; bits++) {
            const size_t *expected = NULL;
            for (size_t choice = 0; choice < 6; choice++) {
                if (before_in_order(bits, permutations[choice])
                    && (expected == NULL || order_precedes(permutations[choice], expected, keys[keyset], ordinals[keyset]))) {
                    expected = permutations[choice];
                }
            }
            for (size_t admission = 0; admission < 6; admission++) {
                for (size_t reverse = 0; reverse < 2; reverse++) {
                    setup_test();
                    const SourceText *source = NULL;
                    SourceStatus loaded = source_manager_add(&sources, "graph", (const unsigned char *)"123456789", 9, &source);
                    assert(loaded == SOURCE_OK);
                    DependencyGraph graph;
                    dependency_graph_init(&graph, sources.allocator);
                    GraphVertex *vertices[3];
                    for (size_t index = 0; index < 3; index++) {
                        size_t vertex = permutations[admission][index];
                        vertices[vertex] = dependency_vertex(&graph, &identities[vertex], keys[keyset][vertex],
                            ordinals[keyset][vertex], (SourceSpan){source, vertex, vertex + 1});
                        assert(vertices[vertex] != NULL);
                    }
                    for (size_t step = 0; step < 9; step++) {
                        size_t edge = reverse ? 8 - step : step;
                        if ((bits & (1u << edge)) == 0) { continue; }
                        bool added = dependency_edge(&graph, vertices[edge / 3], vertices[edge % 3],
                            (SourceSpan){source, edge, edge + 1});
                        assert(added);
                    }
                    GraphStatus status = dependency_order(&graph);
                    assert(status == (expected == NULL ? GRAPH_CYCLE : GRAPH_OK));
                    if (expected != NULL) {
                        GraphVertex *actual = graph.order;
                        for (size_t index = 0; index < 3; index++) {
                            assert(actual == vertices[expected[index]]);
                            actual = actual->order_next;
                        }
                        assert(actual == NULL && graph.valid);
                    } else {
                        assert(!graph.valid && diagnostic_valid(&graph.error));
                        size_t first = graph.error.primary.span.start / 3;
                        size_t owner = first;
                        for (size_t index = 0; index < graph.error.related_count; index++) {
                            const DiagnosticRelated *note = &graph.error.related[index];
                            SourceSpan site = note->location.span;
                            assert(site.source == source && site.end == site.start + 1 && site.start < 9);
                            size_t edge = site.start;
                            assert(edge / 3 == owner && (bits & (1u << edge)));
                            assert(strcmp(note->name, keys[keyset][edge % 3]) == 0);
                            owner = edge % 3;
                        }
                        assert(graph.error.related_count > 0 && owner == first);
                    }
                    dependency_graph_destroy(&graph);
                    destroy_test();
                }
            }
        }
    }
}
