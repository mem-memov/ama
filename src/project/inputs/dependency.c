/**
 * @file
 * @brief Link exact live input consumers without duplicate edges or cross-owner references.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) represents dependency relations explicitly rather than inferring them from filenames.
 */
#include "internal.h"
SourceStatus project_inputs_depend(ProjectInputs *inputs, ProjectInput *consumer, ProjectInput *dependency) {
    if (inputs == NULL || inputs->sealed || consumer == NULL || dependency == NULL
        || consumer->owner != inputs || dependency->owner != inputs) { return SOURCE_INVALID_ARGUMENT; }
    for (ProjectInputEdge *edge = consumer->first; edge != NULL; edge = edge->next) {
        if (edge->target == dependency) { return SOURCE_OK; }
    }
    ProjectInputEdge *edge = source_arena_allocate(&inputs->arena, sizeof(*edge));
    if (edge == NULL) { return SOURCE_OUT_OF_MEMORY; }
    *edge = (ProjectInputEdge){consumer->first, dependency};
    consumer->first = edge;
    return SOURCE_OK;
}
