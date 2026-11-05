#include "../common.c"

static void it_orders_dependencies_before_callers(void) {
    setup_test();
    bool added = graph_add_dependency(graph, "main", "middle", &error) &&
                 graph_add_dependency(graph, "middle", "leaf", &error);
    bool sorted = graph_sort(graph, order, &error);
    assert(added && sorted);
    assert(strcmp(graph_path(graph, order[0]), "leaf") == 0);
    assert(strcmp(graph_path(graph, order[1]), "middle") == 0);
    assert(strcmp(graph_path(graph, order[2]), "main") == 0);
    destroy_test();
}

static void it_deduplicates_edges_and_keeps_isolated_nodes(void) {
    setup_test();
    bool added = graph_add_dependency(graph, "a", "b", &error) &&
                 graph_add_dependency(graph, "a", "b", &error);
    int isolated = graph_add_node(graph, "isolated", &error);
    bool sorted = graph_sort(graph, order, &error);
    assert(added && isolated >= 0 && sorted && graph->count == 3);
    assert(graph->nodes[0].count == 1);
    assert(graph_has_node(graph, "isolated"));
    assert(graph_path(graph, -1) == NULL && graph_path(graph, 3) == NULL);
    destroy_test();
}

static void it_reports_a_cycle_as_a_value(void) {
    setup_test();
    bool added = graph_add_dependency(graph, "a", "b", &error) &&
                 graph_add_dependency(graph, "b", "a", &error);
    bool sorted = graph_sort(graph, order, &error);
    assert(added && !sorted && error.code == ERROR_CYCLE);
    assert(strstr(error.message, "a ->") != NULL);
    assert(strstr(error.message, "b ->") != NULL);
    destroy_test();
}

static void it_can_sort_again_after_reinitializing_a_cyclic_graph(void) {
    setup_test();
    bool added = graph_add_dependency(graph, "a", "a", &error);
    bool cyclic = !graph_sort(graph, order, &error);
    assert(added && cyclic);
    graph_init(graph);
    error_clear(&error);
    int node = graph_add_node(graph, "fresh", &error);
    bool sorted = graph_sort(graph, order, &error);
    assert(node == 0 && sorted && order[0] == 0 && error.code == ERROR_NONE);
    sorted = graph_sort(graph, order, &error);
    assert(sorted && order[0] == 0);
    destroy_test();
}

void run(void) {
    it_orders_dependencies_before_callers();
    it_deduplicates_edges_and_keeps_isolated_nodes();
    it_reports_a_cycle_as_a_value();
    it_can_sort_again_after_reinitializing_a_cyclic_graph();
}

int main(void) {
    run();
    printf("unit/graph/order: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
