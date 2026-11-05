#include "graph/graph.h"
#include <string.h>

void graph_init(Graph* graph) {
    graph->node_count = 0;
}

static int find_node(const Graph* graph, const char* path) {
    for (int index = 0; index < graph->node_count; index++)
        if (!strcmp(graph->nodes[index].path, path)) return index;
    return -1;
}

bool graph_has_node(const Graph* graph, const char* path) {
    return find_node(graph, path) >= 0;
}

int graph_add_node(Graph* graph, const char* path, Error* error) {
    int index = find_node(graph, path);
    if (index >= 0) return index;
    if (graph->node_count == MAX_MODULES || strlen(path) >= MAX_MODULE_PATH_LENGTH) {
        error_set(error, ERROR_LIMIT, 0, "too many modules or module path too long");
        return -1;
    }
    index = graph->node_count++;
    GraphNode* node = &graph->nodes[index];
    strcpy(node->path, path);
    node->dependency_count = 0;
    return index;
}

bool graph_add_dependency(Graph* graph, const char* caller, const char* callee, Error* error) {
    int caller_index = graph_add_node(graph, caller, error);
    if (caller_index < 0) return false;
    int callee_index = graph_add_node(graph, callee, error);
    if (callee_index < 0) return false;
    GraphNode* node = &graph->nodes[caller_index];
    for (int index = 0; index < node->dependency_count; index++)
        if (node->dependencies[index] == callee_index) return true;
    if (node->dependency_count == MAX_MODULES)
        return error_set(error, ERROR_LIMIT, 0, "too many dependencies from '%s'", caller);
    node->dependencies[node->dependency_count++] = callee_index;
    return true;
}

typedef enum {
    NODE_NOT_VISITED,
    NODE_VISITING,
    NODE_VISITED
} NodeVisitState;

typedef struct {
    const Graph* graph;
    NodeVisitState node_states[MAX_MODULES];
    int traversal_path[MAX_MODULES];
    int path_length;
    int* dependency_order;
    int ordered_node_count;
    Error* error;
} GraphTraversal;

static bool report_cycle(GraphTraversal* traversal, int node_index) {
    error_set(traversal->error, ERROR_CYCLE, 0, "recursion is forbidden. Cycle detected:\n");
    bool in_cycle = false;
    for (int index = 0; index <= traversal->path_length; index++) {
        int dependency_index = index == traversal->path_length ? node_index : traversal->traversal_path[index];
        if (dependency_index == node_index) in_cycle = true;
        if (in_cycle) {
            size_t message_length = strlen(traversal->error->message);
            snprintf(traversal->error->message + message_length, sizeof(traversal->error->message) - message_length,
                "  %s%s\n", traversal->graph->nodes[dependency_index].path,
                index == traversal->path_length ? " (back to start)" : " ->");
        }
    }
    return false;
}

static bool visit_dependencies(GraphTraversal* traversal, int node_index) {
    traversal->node_states[node_index] = NODE_VISITING;
    traversal->traversal_path[traversal->path_length++] = node_index;
    const GraphNode* current_node = &traversal->graph->nodes[node_index];
    for (int index = 0; index < current_node->dependency_count; index++) {
        int dependency_index = current_node->dependencies[index];
        if (traversal->node_states[dependency_index] == NODE_VISITING) return report_cycle(traversal, dependency_index);
        if (traversal->node_states[dependency_index] == NODE_NOT_VISITED && !visit_dependencies(traversal, dependency_index)) return false;
    }
    traversal->path_length--;
    traversal->node_states[node_index] = NODE_VISITED;
    traversal->dependency_order[traversal->ordered_node_count++] = node_index;
    return true;
}

bool graph_sort(const Graph* graph, int* dependency_order, Error* error) {
    GraphTraversal traversal = {.graph = graph, .dependency_order = dependency_order, .error = error};
    for (int index = 0; index < graph->node_count; index++)
        if (traversal.node_states[index] == NODE_NOT_VISITED && !visit_dependencies(&traversal, index)) return false;
    return true;
}

const char* graph_path(const Graph* graph, int index) {
    return index < 0 || index >= graph->node_count ? NULL : graph->nodes[index].path;
}
