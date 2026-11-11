#include <stdint.h>
#include "internal.h"

GraphStatus dependency_cycle(DependencyGraph *graph) {
    GraphVertex *cycle = graph->first;
    while (cycle->ordered) {
        cycle = cycle->next;
    }
    while (!cycle->visited) {
        cycle->visited = true;
        cycle = dependency_blocked(cycle)->target;
    }
    size_t count = 0;
    GraphVertex *vertex = cycle;
    do {
        if (count >= SIZE_MAX / sizeof(DiagnosticRelated)) {
            dependency_memory_failure(graph);
            return graph->status;
        }
        count++;
        vertex = dependency_blocked(vertex)->target;
    } while (vertex != cycle);
    DiagnosticRelated *notes = source_arena_allocate(&graph->arena, count * sizeof(*notes));
    if (notes == NULL) {
        dependency_memory_failure(graph);
        return graph->status;
    }
    vertex = cycle;
    for (size_t index = 0; index < count; index++) {
        GraphEdge *edge = dependency_blocked(vertex);
        notes[index] = (DiagnosticRelated){{.span = edge->site},
            "requires earlier construction", edge->target->module_path};
        vertex = edge->target;
    }
    graph->status = GRAPH_CYCLE;
    graph->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
        .message = "Dependency ordering contains a cycle", .subject = cycle->module_path,
        .primary = {.span = dependency_blocked(cycle)->site}, .related = notes, .related_count = count};
    return graph->status;
}
