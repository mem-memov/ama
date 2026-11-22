/** @file
 * @brief Solve finite destination-version constraints without recursive expansion.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves explicit placements and refuses conflicting derived destinations.
 * Each changing pass labels at least one previously unknown unit, so at most N
 * such passes occur even when dependencies cycle. This is an exact least closure.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** Grow the known-label set monotonically and retain the first exact conflict. */
static SourceVersionStatus propagate(const SourceVersionUnit *units, size_t count,
    const SourceVersionFlow *flows, size_t flow_count, const char **labels, SourceVersionIssue *issue) {
    bool changed;
    do {
        changed = false;
        for (size_t i = 0; i < flow_count; i++) {
            size_t from = flows[i].requester, to = flows[i].target;
            if (units[to].fixed_version || !labels[from]) { continue; }
            if (!labels[to]) { labels[to] = labels[from]; changed = true; }
            else if (strcmp(labels[to], labels[from])) {
                *issue = (SourceVersionIssue){to, i, labels[to], labels[from]};
                return SOURCE_VERSION_CONFLICT;
            }
        }
    } while (changed);
    for (size_t i = 0; i < count; i++) {
        if (!labels[i]) { issue->unit = i; return SOURCE_VERSION_UNRESOLVED; }
    }
    return SOURCE_VERSION_OK;
}
SourceVersionStatus source_version_plan(SourceArena *arena, const SourceVersionUnit *units,
    size_t count, const SourceVersionFlow *flows, size_t flow_count,
    const SourceVersionAssignment **output, SourceVersionIssue *issue) {
    if (!issue) { return SOURCE_VERSION_INVALID; }
    *issue = (SourceVersionIssue){0};
    if (!arena || !output || !source_version_plan_valid(units, count, flows, flow_count)) {
        return SOURCE_VERSION_INVALID;
    }
    if (!count) { *output = NULL; return SOURCE_VERSION_OK; }
    if (count > SIZE_MAX / sizeof(const char *) || count > SIZE_MAX / sizeof(SourceVersionAssignment)) {
        return SOURCE_VERSION_MEMORY;
    }
    const char **labels = source_arena_allocate(arena, count * sizeof(*labels));
    if (!labels) { return SOURCE_VERSION_MEMORY; }
    for (size_t i = 0; i < count; i++) { labels[i] = units[i].fixed_version; }
    SourceVersionStatus status = propagate(units, count, flows, flow_count, labels, issue);
    if (status != SOURCE_VERSION_OK) { return status; }
    SourceVersionAssignment *assignments = source_arena_allocate(arena, count * sizeof(*assignments));
    if (!assignments) { return SOURCE_VERSION_MEMORY; }
    for (size_t i = 0; i < count; i++) {
        const char *version = source_arena_copy(arena, labels[i]);
        if (!version) { return SOURCE_VERSION_MEMORY; }
        assignments[i] = (SourceVersionAssignment){units[i].original, version};
    }
    *output = assignments;
    return SOURCE_VERSION_OK;
}
