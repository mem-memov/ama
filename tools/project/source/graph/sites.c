/** @file
 * @brief Validate exported occurrence provenance through explicit successful source edit results.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps implicit constructs and constant operand occurrences exact, while
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates these offset proofs from final captured-byte and publication checks.
 */
#include <string.h>
#include "sites.h"
/** Independent accepted response captures have different owners but exact path/offset identities. */
static bool same_site(SourceSpan expected, SourceSpan actual) {
    return source_span_valid(expected) && source_span_valid(actual)
        && expected.start == actual.start && expected.end == actual.end
        && !strcmp(source_path(expected.source), source_path(actual.source));
}
SourceSitesStatus source_graph_sites(const DeliverySourceDefinition *original,
    const DeliverySourceDefinition *final, const SourceCloneMap *map,
    const char *const *removed, size_t count, const SourceGraphPreparedText *prepared, SourceSitesIssue *issue) {
    if (!issue) { return SOURCE_SITES_INVALID; }
    *issue = (SourceSitesIssue){0};
    for (const SourceGraphPreparedText *row = prepared; row; row = row->next) {
        if (!row->edits || !row->edits->valid) { issue->prepared = row; return SOURCE_SITES_INVALID; }
        for (const SourceGraphPreparedText *previous = prepared; previous != row; previous = previous->next) {
            if (previous->edits->original == row->edits->original) { issue->prepared = row; return SOURCE_SITES_INVALID; }
        }
    }
    issue->binding_status = source_graph_bindings(original, final, map, removed, count, &issue->binding);
    if (issue->binding_status != SOURCE_BINDINGS_OK) { return SOURCE_SITES_BINDINGS; }
    for (const DeliverySourceDefinition *now = final; now; now = now->next) {
        const SourceCloneMapping *copy = map->first;
        while (copy && strcmp(copy->identity, now->identity)) { copy = copy->next; }
        const char *identity = copy ? copy->original->logical_path : now->identity;
        const DeliverySourceDefinition *old = original;
        while (old && strcmp(old->identity, identity)) { old = old->next; }
        issue->binding = (SourceBindingsIssue){.original = old, .final = now, .clone = copy};
        const DeliverySourceDependency *old_edge = old->dependencies, *now_edge = now->dependencies;
        for (; old_edge; old_edge = old_edge->next, now_edge = now_edge->next) {
            SourceSpan expected = old_edge->origin;
            issue->original_edge = old_edge; issue->final_edge = now_edge;
            issue->prepared = NULL;
            if (copy) {
                const SourceGraphPreparedText *text = prepared;
                while (text && text->edits->original != old_edge->origin.source) { text = text->next; }
                issue->prepared = text;
                if (!text) { return SOURCE_SITES_MISSING; }
                if (!source_text_edits_project(text->edits, old_edge->origin, &expected)) { return SOURCE_SITES_CHANGED; }
            }
            if (!same_site(expected, now_edge->origin)) { return SOURCE_SITES_CHANGED; }
        }
    }
    *issue = (SourceSitesIssue){0}; return SOURCE_SITES_OK;
}
