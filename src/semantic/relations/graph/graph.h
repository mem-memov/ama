#ifndef AMA_DEPENDENCY_ORDER_GRAPH_H
#define AMA_DEPENDENCY_ORDER_GRAPH_H

#include "diagnostics/record/diagnostic.h"
#include "source/memory/arena.h"

typedef enum { GRAPH_OK, GRAPH_CYCLE, GRAPH_OUT_OF_MEMORY } GraphStatus;
typedef struct GraphVertex GraphVertex;
typedef struct GraphEdge {
    struct GraphEdge *next;
    GraphVertex *target;
    SourceSpan site;
} GraphEdge;
struct GraphVertex {
    GraphVertex *next;
    GraphVertex *order_next;
    const void *value;
    const char *module_path;
    size_t ordinal;
    SourceSpan site;
    GraphEdge *first;
    bool ordered;
    bool visited;
};
typedef struct {
    SourceArena arena;
    GraphVertex *first;
    GraphVertex *last;
    GraphVertex *order;
    GraphVertex *order_last;
    GraphStatus status;
    Diagnostic error;
    bool attempted;
    bool valid;
} DependencyGraph;

/* Borrow values, paths and spans until destruction. Add finite vertices/edges,
 * then order once. An edge owner->target means target must precede owner.
 * Distinct values have distinct (module path, ordinal) keys.
 * Stable order compares full module path bytes, then declaration ordinal.
 * Successful order includes every vertex once; failure retains an actual cycle.
 * Partial allocations remain owned; graph destruction never dereferences values. */
void dependency_graph_init(DependencyGraph *graph, SourceAllocator allocator);
void dependency_graph_destroy(DependencyGraph *graph);
GraphVertex *dependency_vertex(DependencyGraph *graph, const void *value,
    const char *module_path, size_t ordinal, SourceSpan site);
bool dependency_edge(DependencyGraph *graph, GraphVertex *owner,
    GraphVertex *target, SourceSpan site);
GraphStatus dependency_order(DependencyGraph *graph);

#endif
