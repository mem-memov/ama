/**
 * @file
 * @brief Own graph vertices/edges with borrowed semantic identities and source sites.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * Allocate/link complete records only; arena destruction never dereferences borrowed semantic values.
 */
#include <assert.h>
#include "internal.h"

void dependency_graph_init(DependencyGraph *graph, SourceAllocator allocator) {
    *graph = (DependencyGraph){0};
    source_arena_init(&graph->arena, allocator);
}

void dependency_graph_destroy(DependencyGraph *graph) {
    source_arena_destroy(&graph->arena);
    *graph = (DependencyGraph){0};
}

bool dependency_memory_failure(DependencyGraph *graph) {
    graph->status = GRAPH_OUT_OF_MEMORY;
    graph->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
        .message = "Dependency ordering could not allocate storage"};
    return false;
}

GraphVertex *dependency_vertex(DependencyGraph *graph, const void *value,
    const char *module_path, size_t ordinal, SourceSpan site) {
    assert(!graph->attempted);
    for (GraphVertex *vertex = graph->first; vertex != NULL; vertex = vertex->next) {
        if (vertex->value == value) {
            return vertex;
        }
    }
    GraphVertex *vertex = source_arena_allocate(&graph->arena, sizeof(*vertex));
    if (vertex == NULL) {
        dependency_memory_failure(graph);
        return NULL;
    }
    *vertex = (GraphVertex){.value = value, .module_path = module_path, .ordinal = ordinal, .site = site};
    if (graph->last == NULL) {
        graph->first = vertex;
    } else {
        graph->last->next = vertex;
    }
    graph->last = vertex;
    return vertex;
}

bool dependency_edge(DependencyGraph *graph, GraphVertex *owner, GraphVertex *target, SourceSpan site) {
    assert(!graph->attempted);
    if (owner == NULL || target == NULL) {
        return dependency_memory_failure(graph);
    }
    GraphEdge *edge = source_arena_allocate(&graph->arena, sizeof(*edge));
    if (edge == NULL) {
        return dependency_memory_failure(graph);
    }
    *edge = (GraphEdge){owner->first, target, site};
    owner->first = edge;
    return true;
}
