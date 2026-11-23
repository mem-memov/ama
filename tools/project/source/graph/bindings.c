/** @file
 * @brief Validate copied and surviving edge targets simultaneously, including cyclic source graphs.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * defines target substitution through one injective C, not runtime reachability
 * or sequential source rewriting. All loops visit finite exported lists only.
 */
#include <string.h>
#include "bindings.h"
/** Checked exports own unique complete definition names. */
static const DeliverySourceDefinition *definition(const DeliverySourceDefinition *list, const char *identity) {
    while (list && strcmp(list->identity, identity)) { list = list->next; }
    return list;
}
/** Membership includes explicitly removed draft names without inventing definitions. */
static bool removed_name(const char *identity, const char *const *removed, size_t count) {
    for (size_t index = 0; index < count; index++) { if (!strcmp(identity, removed[index])) { return true; } }
    return false;
}
/** Find by complete original identity for edge substitution, or complete copy identity for final ownership. */
static const SourceCloneMapping *mapping(const SourceCloneMap *map, const char *identity, bool original) {
    const SourceCloneMapping *row = map->first;
    while (row && strcmp(original ? row->original->logical_path : row->identity, identity)) { row = row->next; }
    return row;
}
/** Nullable normalized member identity is part of the binding, not operand text. */
static bool member(const char *left, const char *right) {
    return left && right ? !strcmp(left, right) : left == right;
}
/** Preserve ordered occurrences; compute each expected target from the same sealed map. */
static SourceBindingsStatus edges(const SourceCloneMap *map, const char *const *removed,
    size_t count, SourceBindingsIssue *issue) {
    const DeliverySourceDependency *old = issue->original->dependencies, *now = issue->final->dependencies;
    while (old || now) {
        const SourceCloneMapping *target = old && issue->clone ? mapping(map, old->target, true) : NULL;
        const char *expected = old ? target ? target->identity : old->target : NULL;
        if (old && !target && removed_name(old->target, removed, count)) {
            issue->original_edge = old; issue->final_edge = now; issue->expected = expected;
            return SOURCE_BINDINGS_REMOVED_TARGET;
        }
        if (!old || !now || strcmp(expected, now->target) || old->implicit != now->implicit || !member(old->member, now->member)) {
            issue->original_edge = old; issue->final_edge = now; issue->expected = expected;
            return SOURCE_BINDINGS_EDGE;
        }
        old = old->next; now = now->next;
    }
    return SOURCE_BINDINGS_OK;
}
SourceBindingsStatus source_graph_bindings(const DeliverySourceDefinition *original,
    const DeliverySourceDefinition *final, const SourceCloneMap *map,
    const char *const *removed, size_t count, SourceRegistryRetirement retired, SourceBindingsIssue *issue) {
    if (!issue) { return SOURCE_BINDINGS_INVALID; }
    *issue = (SourceBindingsIssue){0};
    if (!map || !map->ready || (count && !removed) || (retired.count && !retired.names)) { return SOURCE_BINDINGS_INVALID; }
    for (size_t index = 0; index < count; index++) { if (!removed[index]) { return SOURCE_BINDINGS_INVALID; } }
    for (size_t index = 0; index < retired.count; index++) { if (!retired.names[index]) { return SOURCE_BINDINGS_INVALID; } }
    for (const DeliverySourceDefinition *now = final; now; now = now->next) {
        const DeliverySourceDefinition *old = definition(original, now->identity);
        if (old && (removed_name(old->identity, removed, count) || removed_name(old->identity, retired.names, retired.count))) { old = NULL; }
        const SourceCloneMapping *copy = mapping(map, now->identity, false);
        *issue = (SourceBindingsIssue){.original = old, .final = now, .clone = copy, .expected = now->identity};
        if (old && copy) { return SOURCE_BINDINGS_COLLISION; }
        if (copy) { old = definition(original, copy->original->logical_path); issue->original = old; }
        if (!old) { return SOURCE_BINDINGS_EXTRA; }
        if (old->kind != now->kind) { return SOURCE_BINDINGS_KIND; }
        SourceBindingsStatus status = edges(map, removed, count, issue);
        if (status != SOURCE_BINDINGS_OK) { return status; }
    }
    for (const DeliverySourceDefinition *old = original; old; old = old->next) {
        if (!removed_name(old->identity, removed, count) && !removed_name(old->identity, retired.names, retired.count)
            && !definition(final, old->identity)) {
            *issue = (SourceBindingsIssue){.original = old, .expected = old->identity};
            return SOURCE_BINDINGS_MISSING;
        }
    }
    for (const SourceCloneMapping *copy = map->first; copy; copy = copy->next) {
        const DeliverySourceDefinition *old = definition(original, copy->original->logical_path);
        if (old && !definition(final, copy->identity)) {
            *issue = (SourceBindingsIssue){.original = old, .clone = copy, .expected = copy->identity};
            return SOURCE_BINDINGS_MISSING;
        }
    }
    *issue = (SourceBindingsIssue){0};
    return SOURCE_BINDINGS_OK;
}
