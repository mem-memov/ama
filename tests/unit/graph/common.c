#ifndef AMA_TESTS_GRAPH_COMMON_C
#define AMA_TESTS_GRAPH_COMMON_C
#include "../common.c"
#include "graph/graph.h"
static Graph* graph;
static int dependency_order[MAX_MODULES];
static void setup_test(void) {
    test_count++;
    error_clear(&error);
    graph = malloc(sizeof(*graph));
    assert(graph != NULL);
    graph_init(graph);
}
static void destroy_test(void) {
    free(graph);
    graph = NULL;
}
#endif
