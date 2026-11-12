/**
 * @file
 * @brief Declare exact blocked-edge, cycle and allocation-failure graph helpers.
 *
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * A stalled finite graph supplies a real closed prerequisite walk, not a timeout or guessed cycle.
 */
#ifndef AMA_DEPENDENCY_GRAPH_INTERNAL_H
#define AMA_DEPENDENCY_GRAPH_INTERNAL_H

#include "graph.h"

/** Record graph allocation failure without allocating a diagnostic fallback.
 */
bool dependency_memory_failure(DependencyGraph *graph);
/** Return one exact edge to an unordered prerequisite, or null when the vertex is ready.
 */
GraphEdge *dependency_blocked(const GraphVertex *vertex);
/** Require a stalled finite graph with unfinished vertices and no ready vertex; follow blocked edges to an actual closed cycle and preserve every cycle edge site.
 */
GraphStatus dependency_cycle(DependencyGraph *graph);

#endif
