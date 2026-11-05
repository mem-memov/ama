#include "../common.c"

static void it_orders_dependencies_before_callers(void) {
    setup_test();
    bool added = graph_add_dependency(graph, "main", "middle", &error) &&
                    graph_add_dependency(graph, "middle", "leaf", &error);
    bool sorted = graph_sort(graph, dependency_order, &error);
    assert(added && sorted);
    assert(strcmp(graph_path(graph, dependency_order[0]), "leaf") == 0);
    assert(strcmp(graph_path(graph, dependency_order[1]), "middle") == 0);
    assert(strcmp(graph_path(graph, dependency_order[2]), "main") == 0);
    destroy_test();
}

static void it_deduplicates_edges_and_keeps_isolated_nodes(void) {
    setup_test();
    bool added = graph_add_dependency(graph, "first_module", "second_module", &error) &&
                    graph_add_dependency(graph, "first_module", "second_module", &error);
    int isolated = graph_add_node(graph, "isolated", &error);
    bool sorted = graph_sort(graph, dependency_order, &error);
    assert(added && isolated >= 0 && sorted && graph->node_count == 3);
    assert(graph->nodes[0].dependency_count == 1);
    assert(graph_has_node(graph, "isolated"));
    assert(graph_path(graph, -1) == NULL && graph_path(graph, 3) == NULL);
    destroy_test();
}

static void it_reports_a_cycle_as_a_value(void) {
    setup_test();
    bool added = graph_add_dependency(graph, "first_module", "second_module", &error) &&
                    graph_add_dependency(graph, "second_module", "first_module", &error);
    bool sorted = graph_sort(graph, dependency_order, &error);
    assert(added && !sorted && error.error_code == ERROR_CYCLE);
    assert(strstr(error.message, "first_module ->") != NULL);
    assert(strstr(error.message, "second_module ->") != NULL);
    destroy_test();
}

static void it_can_sort_again_after_reinitializing_a_cyclic_graph(void) {
    setup_test();
    bool added = graph_add_dependency(graph, "first_module", "first_module", &error);
    bool cyclic = !graph_sort(graph, dependency_order, &error);
    assert(added && cyclic);
    graph_init(graph);
    error_clear(&error);
    int node = graph_add_node(graph, "fresh", &error);
    bool sorted = graph_sort(graph, dependency_order, &error);
    assert(node == 0 && sorted && dependency_order[0] == 0 && error.error_code == ERROR_NONE);
    sorted = graph_sort(graph, dependency_order, &error);
    assert(sorted && dependency_order[0] == 0);
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
    printf("unit/graph/order: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
