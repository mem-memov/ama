/**
 * @file
 * @brief Own a finite prerequisite graph with deterministic order and concrete cycle evidence.
 *
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * Edges owner-to-target require target first; full supported module bytes and declaration ordinals order ready vertices.
 */
#ifndef AMA_DEPENDENCY_ORDER_GRAPH_H
#define AMA_DEPENDENCY_ORDER_GRAPH_H

#include "diagnostics/record/diagnostic.h"
#include "source/memory/arena.h"

/** Graph outcome distinguishes concrete cycle evidence from allocation exhaustion. */
typedef enum { GRAPH_OK, GRAPH_CYCLE, GRAPH_OUT_OF_MEMORY } GraphStatus;
typedef struct GraphVertex GraphVertex;
/** Graph-owned prerequisite edge borrowing its original semantic source site. */
typedef struct GraphEdge {
    struct GraphEdge *next;
    GraphVertex *target;
    SourceSpan site;
} GraphEdge;
/** Graph-owned exact borrowed value/key with separate insertion/order links and finite ordering/cycle state. */
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
/** Arena owner of finite vertices/edges/cycle notes; valid order is published only after one complete successful attempt. */
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
/** Initialize an empty arena-owned graph; values/path/source owners remain borrowed through destruction.
 */
void dependency_graph_init(DependencyGraph *graph, SourceAllocator allocator);
/** Release graph records/notes without dereferencing borrowed values; all vertex/order views expire.
 */
void dependency_graph_destroy(DependencyGraph *graph);
/** Before ordering, reuse the exact value-pointer identity or append one complete vertex. Distinct values must have distinct stable module/ordinal keys.
 */
GraphVertex *dependency_vertex(DependencyGraph *graph, const void *value,
    const char *module_path, size_t ordinal, SourceSpan site);
/** Before ordering, append owner-to-prerequisite edge with exact source site. Null endpoints/allocation failure set explicit graph failure.
 */
bool dependency_edge(DependencyGraph *graph, GraphVertex *owner,
    GraphVertex *target, SourceSpan site);
/** Order once by least full-key ready vertex or retain a concrete cycle.
 * @return Explicit cycle/allocation/success; only GRAPH_OK publishes valid and every vertex exactly once.
 */
GraphStatus dependency_order(DependencyGraph *graph);

#endif
