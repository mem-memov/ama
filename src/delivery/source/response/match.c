/** @file
 * @brief Compare exact finite capture sets before admitting compiler graph facts.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects changed, missing or ambiguous input observations without filesystem fallback.
 */
#include <string.h>
#include "match.h"
/** Nullable paths compare by bytes, including non-UTF-8 physical names under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static bool path_equal(const char *left, const char *right) {
    return left && right ? !strcmp(left, right) : left == right;
}
/** Presence, path and complete binary content are independent observations under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static bool text_equal(const SourceText *left, const SourceText *right) {
    if (!left || !right) { return left == right; }
    return path_equal(source_path(left), source_path(right))
        && source_length(left) == source_length(right)
        && !memcmp(source_bytes(left), source_bytes(right), source_length(left));
}
/** Exactly one prepared companion row supplies authority; unobserved ordinary
 * companions stay opaque under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static bool companions_match(const DeliveryResponseSource *source, const DeliverySourceCompanions *rows) {
    const DeliverySourceCompanions *found = NULL;
    for (const DeliverySourceCompanions *row = rows; row; row = row->next) {
        if (!row->identity) { return false; }
        if (strcmp(source->identity, row->identity)) { continue; }
        if (found) { return false; }
        found = row;
    }
    return found && (!source->native_captured || (path_equal(source->native_body, found->body)
        && path_equal(source->native_header, found->header) && text_equal(source->native_abi, found->abi)));
}
/** Equal cardinality plus singleton membership proves finite set equality under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
static bool sources_match(const DeliverySourceResponse *response, const DeliverySourceSnapshotView *view) {
    size_t count = 0;
    for (const DeliveryResponseSource *source = response->sources; source; source = source->next) {
        if (count == view->count) { return false; }
        count++;
        const ModuleSnapshotEntry *found = NULL;
        for (size_t index = 0; index < view->count; index++) {
            const ModuleSnapshotEntry *entry = &view->entries[index];
            if (!entry->logical_path || !entry->text) { return false; }
            if (strcmp(source->identity, entry->logical_path)) { continue; }
            if (found) { return false; }
            found = entry;
        }
        if (!found || !source->configuration_captured || !text_equal(source->source, found->text)
            || !text_equal(source->configuration, found->configuration)
            || !companions_match(source, view->companions)) { return false; }
    }
    if (count != view->count) { return false; }
    for (const DeliverySourceCompanions *row = view->companions; row; row = row->next) {
        bool found = false;
        for (const DeliveryResponseSource *source = response->sources; source; source = source->next) {
            if (row->identity && !strcmp(row->identity, source->identity)) { found = true; break; }
        }
        if (!found) { return false; }
    }
    return true;
}
bool delivery_source_response_matches(const DeliverySourceResponse *response, const DeliverySourceSnapshotView *view) {
    if (!response || !response->valid || !view || (view->count && !view->entries)
        || !sources_match(response, view)) { return false; }
    size_t expected = 0, actual = 0;
    for (const DeliverySnapshotVersion *version = view->versions; version; version = version->next) {
        if (!version->identity || !version->directory || !version->configuration) { return false; }
        expected++;
    }
    for (const DeliveryResponseVersion *version = response->versions; version; version = version->next) {
        if (actual == expected) { return false; }
        actual++;
        const DeliverySnapshotVersion *found = NULL;
        for (const DeliverySnapshotVersion *row = view->versions; row; row = row->next) {
            if (strcmp(row->identity, version->identity)) { continue; }
            if (found) { return false; }
            found = row;
        }
        if (!found || !path_equal(found->directory, version->directory)
            || !text_equal(found->configuration, version->configuration)) { return false; }
    }
    return actual == expected;
}
