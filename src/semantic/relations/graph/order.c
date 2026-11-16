/**
 * @file
 * @brief Order ready vertices by exact stable keys or report a real dependency cycle.
 *
 * [COMP-05](../../../../spec/implementation/compiler.md#comp-05)
 * Repeatedly choose the least full-key ready vertex; no hash order or catalog traversal order selects output.
 */
#include <assert.h>
#include <string.h>
#include "internal.h"

GraphEdge *dependency_blocked(const GraphVertex *vertex) {
    for (GraphEdge *edge = vertex->first; edge != NULL; edge = edge->next) {
        if (!edge->target->ordered) {
            return edge;
        }
    }
    return NULL;
}

/** Compare complete borrowed module path bytes, then declaration ordinal; unique keys make the ready choice deterministic.
 * Component contract: [COMP-05](../../../../spec/implementation/compiler.md#comp-05). */
static bool precedes(const GraphVertex *left, const GraphVertex *right) {
    int compared = strcmp(left->module_path, right->module_path);
    return compared < 0 || (compared == 0 && left->ordinal < right->ordinal);
}

GraphStatus dependency_order(DependencyGraph *graph) {
    assert(!graph->attempted);
    graph->attempted = true;
    if (graph->status != GRAPH_OK) {
        return graph->status;
    }
    for (;;) {
        GraphVertex *ready = NULL;
        bool unfinished = false;
        for (GraphVertex *vertex = graph->first; vertex != NULL; vertex = vertex->next) {
            if (!vertex->ordered) {
                unfinished = true;
                if (dependency_blocked(vertex) == NULL && (ready == NULL || precedes(vertex, ready))) {
                    ready = vertex;
                }
            }
        }
        if (!unfinished) {
            graph->valid = true;
            return GRAPH_OK;
        }
        if (ready == NULL) {
            return dependency_cycle(graph);
        }
        ready->ordered = true;
        if (graph->order_last == NULL) {
            graph->order = ready;
        } else {
            graph->order_last->order_next = ready;
        }
        graph->order_last = ready;
    }
}
