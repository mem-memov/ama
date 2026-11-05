#include "graph/graph.h"
#include <string.h>

void graph_init(Graph* graph) {
    graph->count = 0;
}

static int find_node(const Graph* graph, const char* path) {
    for (int i = 0; i < graph->count; i++)
        if (!strcmp(graph->nodes[i].path, path)) return i;
    return -1;
}

bool graph_has_node(const Graph* graph, const char* path) {
    return find_node(graph, path) >= 0;
}

int graph_add_node(Graph* graph, const char* path, Error* error) {
    int index = find_node(graph, path);
    if (index >= 0) return index;
    if (graph->count == MAX_MODULES || strlen(path) >= MAX_PATH_LEN) {
        error_set(error, ERROR_LIMIT, 0, "too many modules or module path too long");
        return -1;
    }
    index = graph->count++;
    GraphNode* node = &graph->nodes[index];
    strcpy(node->path, path);
    node->count = 0;
    return index;
}

bool graph_add_dependency(Graph* graph, const char* caller, const char* callee, Error* error) {
    int a = graph_add_node(graph, caller, error);
    if (a < 0) return false;
    int b = graph_add_node(graph, callee, error);
    if (b < 0) return false;
    GraphNode* node = &graph->nodes[a];
    for (int i = 0; i < node->count; i++)
        if (node->dependencies[i] == b) return true;
    if (node->count == MAX_MODULES)
        return error_set(error, ERROR_LIMIT, 0, "too many dependencies from '%s'", caller);
    node->dependencies[node->count++] = b;
    return true;
}

typedef struct {
    const Graph* graph;
    int colors[MAX_MODULES];
    int path[MAX_MODULES];
    int depth;
    int* order;
    int count;
    Error* error;
} Sort;

static bool report_cycle(Sort* sort, int node) {
    error_set(sort->error, ERROR_CYCLE, 0, "recursion is forbidden. Cycle detected:\n");
    bool in_cycle = false;
    for (int i = 0; i <= sort->depth; i++) {
        int next = i == sort->depth ? node : sort->path[i];
        if (next == node) in_cycle = true;
        if (in_cycle) {
            size_t used = strlen(sort->error->message);
            snprintf(sort->error->message + used, sizeof(sort->error->message) - used,
                     "  %s%s\n", sort->graph->nodes[next].path,
                     i == sort->depth ? " (back to start)" : " ->");
        }
    }
    return false;
}

static bool visit(Sort* sort, int node) {
    sort->colors[node] = 1;
    sort->path[sort->depth++] = node;
    const GraphNode* current = &sort->graph->nodes[node];
    for (int i = 0; i < current->count; i++) {
        int next = current->dependencies[i];
        if (sort->colors[next] == 1) return report_cycle(sort, next);
        if (!sort->colors[next] && !visit(sort, next)) return false;
    }
    sort->depth--;
    sort->colors[node] = 2;
    sort->order[sort->count++] = node;
    return true;
}

bool graph_sort(const Graph* graph, int* order, Error* error) {
    Sort sort = {.graph = graph, .order = order, .error = error};
    for (int i = 0; i < graph->count; i++)
        if (!sort.colors[i] && !visit(&sort, i)) return false;
    return true;
}

const char* graph_path(const Graph* graph, int index) {
    return index < 0 || index >= graph->count ? NULL : graph->nodes[index].path;
}
