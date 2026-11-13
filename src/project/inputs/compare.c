/**
 * @file
 * @brief Compute exact changed-input reachability by a finite monotone fixed point.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) requires rebuilding all affected consumers, including those reached through module cycles.
 */
#include <string.h>
#include "internal.h"

/** Compare dependency sets by category and exact key; insertion order and duplicate requests have no graph meaning. */
static bool contains(const ProjectInput *node, const ProjectInput *target) {
    for (ProjectInputEdge *edge = node->first; edge != NULL; edge = edge->next) {
        if (edge->target->kind == target->kind && strcmp(edge->target->key, target->key) == 0) { return true; }
    }
    return false;
}
/** A direct seed differs in membership, original bytes or dependency-set membership. */
static bool directly_changed(const ProjectInput *node, const ProjectInputs *previous) {
    const ProjectInput *old = project_inputs_find(previous, node->kind, node->key);
    if (old == NULL) { return true; }
    size_t length = node->content == NULL ? 0 : source_length(node->content);
    size_t old_length = old->content == NULL ? 0 : source_length(old->content);
    if (length != old_length || (length != 0 && memcmp(source_bytes(node->content), source_bytes(old->content), length) != 0)) {
        return true;
    }
    for (ProjectInputEdge *edge = node->first; edge != NULL; edge = edge->next) {
        if (!contains(old, edge->target)) { return true; }
    }
    for (ProjectInputEdge *edge = old->first; edge != NULL; edge = edge->next) {
        if (!contains(node, edge->target)) { return true; }
    }
    return false;
}
SourceStatus project_inputs_compare(ProjectInputs *inputs, const ProjectInputs *previous) {
    if (inputs == NULL || !inputs->sealed || (previous != NULL && !previous->sealed)) { return SOURCE_INVALID_ARGUMENT; }
    for (ProjectInput *node = inputs->first; node != NULL; node = node->next) {
        node->changed = directly_changed(node, previous);
    }
    bool progress;
    do {
        progress = false;
        for (ProjectInput *node = inputs->first; node != NULL; node = node->next) {
            if (node->changed) { continue; }
            for (ProjectInputEdge *edge = node->first; edge != NULL; edge = edge->next) {
                if (edge->target->changed) {
                    node->changed = true;
                    progress = true;
                    break;
                }
            }
        }
    } while (progress);
    inputs->compared = true;
    return SOURCE_OK;
}
bool project_inputs_changed(const ProjectInputs *inputs, ProjectInputKind kind, const char *key, bool *output) {
    if (inputs == NULL || !inputs->compared || output == NULL) { return false; }
    const ProjectInput *node = project_inputs_find(inputs, kind, key);
    if (node == NULL) { return false; }
    *output = node->changed;
    return true;
}
