/**
 * @file
 * @brief Validate eager ownership and constant dependency orders as separate graphs.
 *
 * [COMP-03](../../../spec/implementation/compiler.md#comp-03)
 * Loading, construction, initialization and clone relations cannot substitute for one another.
 */
#include <assert.h>
#include "semantic/internal.h"

/** Adapt exact graph ordering/cycle/allocation status into semantic context, preserving graph source evidence and a relation-specific cycle message.
 * Component contract: [COMP-03](../../../spec/implementation/compiler.md#comp-03). */
static bool ordered(SemanticProgram *program, DependencyGraph *graph, const char *message) {
    GraphStatus status = dependency_order(graph);
    if (status == GRAPH_OK) {
        return true;
    }
    program->status = status == GRAPH_OUT_OF_MEMORY ? SEMANTIC_OUT_OF_MEMORY : SEMANTIC_INVALID;
    program->error = graph->error;
    if (status == GRAPH_CYCLE) {
        program->error.message = message;
    }
    return false;
}

SemanticStatus semantic_relations(SemanticProgram *program) {
    assert(program->resolved && !program->relations_attempted);
    program->relations_attempted = true;
    if (!semantic_eager_graph(program)) {
        semantic_memory_failure(program);
    } else if (ordered(program, &program->eager, "Infinite eager ownership construction cycle")) {
        if (!semantic_constant_graph(program)) {
            if (program->status == SEMANTIC_OK) {
                semantic_memory_failure(program);
            }
        } else if (ordered(program, &program->constants, "Shared constant initialization cycle")) {
            program->relations_valid = true;
        }
    }
    return program->status;
}
