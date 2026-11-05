#ifndef AMA_GRAPH_GRAPH_H
#define AMA_GRAPH_GRAPH_H
#include "common/limits.h"
#include "common/diagnostic.h"

typedef struct {
    char path[MAX_PATH_LEN];
    int dependencies[MAX_MODULES];
    int count;
} GraphNode;

typedef struct {
    GraphNode nodes[MAX_MODULES];
    int count;
} Graph;

void graph_init(Graph* graph);
int graph_add_node(Graph* graph, const char* path, Error* error);
bool graph_has_node(const Graph* graph, const char* path);
bool graph_add_dependency(Graph* graph, const char* caller, const char* callee, Error* error);
/* order must hold MAX_MODULES indices. A failed sort leaves it unspecified. */
bool graph_sort(const Graph* graph, int* order, Error* error);
const char* graph_path(const Graph* graph, int index);
#endif
