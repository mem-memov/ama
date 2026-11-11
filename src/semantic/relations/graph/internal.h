#ifndef AMA_DEPENDENCY_GRAPH_INTERNAL_H
#define AMA_DEPENDENCY_GRAPH_INTERNAL_H

#include "graph.h"

bool dependency_memory_failure(DependencyGraph *graph);
GraphEdge *dependency_blocked(const GraphVertex *vertex);
GraphStatus dependency_cycle(DependencyGraph *graph);

#endif
