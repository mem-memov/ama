/** @file
 * @brief Scan every original authored edge and declared input for surviving blockers.
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * checks the original graph before replacement availability; [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * leaves mutation and compiler analysis outside this pure finite predicate.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) gives failed
 * report prefixes operation-local ownership until teardown.
 */
#include "check.h"
#include <string.h>
/** Exact set membership preserves nominal identities and resolved physical keys;
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * permits no guessed subtree/prefix or same-content equivalence here.
 */
static bool contains(const char *const *set, size_t count, const char *key) {
    for (size_t index = 0; index < count; index++) { if (!strcmp(set[index], key)) { return true; } }
    return false;
}
/** Require only complete normalized nonempty keys, not a new path grammar under
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01).
 */
static bool keys(const char *const *set, size_t count) {
    if (count && !set) { return false; }
    for (size_t index = 0; index < count; index++) { if (!set[index] || !*set[index]) { return false; } }
    return true;
}
/** Validate optional original locations without losing CLI-declared input support
 * under [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03).
 */
static bool site(SourceSpan origin) {
    return origin.source ? source_span_valid(origin) : !origin.start && !origin.end;
}
/** Preserve one complete blocking occurrence in original scan order; failed
 * scratch is arena-owned under [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
static bool append(SourceArena *arena, SourceRemovalBlocker ***tail, SourceRemovalBlockKind kind,
    const DeliverySourceDefinition *owner, const char *target, SourceSpan origin) {
    SourceRemovalBlocker *row = source_arena_allocate(arena, sizeof(*row));
    if (!row) { return false; }
    *row = (SourceRemovalBlocker){.kind = kind, .owner = owner, .target = target, .origin = origin};
    **tail = row; *tail = &row->next; return true;
}
SourceRemovalStatus source_removal_check(SourceArena *arena, const DeliverySourceDefinition *definitions,
    const SourceRemovalInputUse *inputs, const char *const *removed, size_t removed_count,
    const char *const *paths, size_t path_count, const SourceRemovalBlocker **output) {
    if (!arena || !output || !keys(removed, removed_count) || !keys(paths, path_count)) { return SOURCE_REMOVAL_INVALID; }
    SourceRemovalBlocker *first = NULL, **tail = &first;
    for (const DeliverySourceDefinition *owner = definitions; owner; owner = owner->next) {
        if (!owner->identity || !*owner->identity) { return SOURCE_REMOVAL_INVALID; }
        bool survives = !contains(removed, removed_count, owner->identity);
        for (const DeliverySourceDependency *edge = owner->dependencies; edge; edge = edge->next) {
            if (!edge->target || !*edge->target || !site(edge->origin)) { return SOURCE_REMOVAL_INVALID; }
            if (survives && contains(removed, removed_count, edge->target) &&
                !append(arena, &tail, SOURCE_REMOVAL_MODULE, owner, edge->target, edge->origin)) { return SOURCE_REMOVAL_OUT_OF_MEMORY; }
        }
    }
    for (const SourceRemovalInputUse *use = inputs; use; use = use->next) {
        const DeliverySourceDefinition *owner = definitions;
        while (owner && owner != use->owner) { owner = owner->next; }
        if (!owner || !use->path || !*use->path || !site(use->origin)) { return SOURCE_REMOVAL_INVALID; }
        if (!contains(removed, removed_count, owner->identity) && contains(paths, path_count, use->path) &&
            !append(arena, &tail, SOURCE_REMOVAL_INPUT, owner, use->path, use->origin)) { return SOURCE_REMOVAL_OUT_OF_MEMORY; }
    }
    *output = first; return first ? SOURCE_REMOVAL_BLOCKED : SOURCE_REMOVAL_OK;
}
