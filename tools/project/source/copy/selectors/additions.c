/** @file
 * @brief Collect exact copied-edge targets uncovered by a preserved fixed permission baseline.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * grants only explicit concrete additions; [PKG-05](../../../../../spec/project/packages.md#pkg-05)
 * and [PKG-09](../../../../../spec/project/packages.md#pkg-09) retain shared coverage/direction semantics.
 */
#include "additions.h"
#include <stdint.h>
#include <string.h>
/** Lookup uses complete canonical identities rather than prefix or content equality. */
static const SourceCloneMapping *clone(const SourceCloneMap *map, const char *name) {
    for (const SourceCloneMapping *row = map->first; row; row = row->next) { if (!strcmp(row->original->logical_path, name)) { return row; } }
    return NULL;
}
/** One-based actual index distinguishes absence from the first canonical entry. */
static size_t actual(const DependencyView *view, const char *name) {
    for (size_t i = 0; i < view->count; i++) { if (!strcmp(view->entries[i].identity->text, name)) { return i + 1; } }
    return 0;
}
/** Validate canonical unique baseline pointer order before constructing its normal
 * sparse root union. Caller has already proved why these roots are authorized. */
static bool baseline_valid(const DependencyView *view, const SourceSelectionRoots *roots) {
    if (!roots || roots->count > view->count || (roots->count && !roots->entries)) { return false; }
    size_t previous = 0;
    for (size_t i = 0; i < roots->count; i++) {
        size_t index = 0;
        for (size_t j = 0; j < view->count; j++) { if (roots->entries[i] == &view->entries[j]) { index = j + 1; break; } }
        if (index <= previous) { return false; } previous = index;
    }
    return true;
}
SourcePermissionStatus source_copy_permission_additions(SourceArena *arena, const SourceCopyClosure *copy,
    const SourceCloneMap *map, const SourcePackageEnvironment *environment, const SourcePackageScope *scope,
    const SourceSelectionRoots *baseline, SourceSelectionRoots *output, SourcePermissionIssue *issue) {
    if (!issue) { return SOURCE_PERMISSION_INVALID; } *issue = (SourcePermissionIssue){0};
    if (!arena || !copy || !copy->valid || !map || !map->ready || map->capture != copy->tree->capture
        || !environment || !environment->valid || !output) { return SOURCE_PERMISSION_INVALID; }
    bool owns_scope = false;
    for (const SourcePackageScope *row = environment->first; row; row = row->next) { if (row == scope) { owns_scope = true; } }
    if (!owns_scope || !baseline_valid(environment->view, baseline)) { return SOURCE_PERMISSION_INVALID; }
    if (baseline->count > SIZE_MAX / sizeof(DependencySelectionRoot)) { return SOURCE_PERMISSION_LIMIT; }
    DependencySelectionRoot *roots = baseline->count ? source_arena_allocate(arena, baseline->count * sizeof(*roots)) : NULL;
    if (baseline->count && !roots) { return SOURCE_PERMISSION_MEMORY; }
    for (size_t i = 0; i < baseline->count; i++) { roots[i] = (DependencySelectionRoot){baseline->entries[i], i + 1 < baseline->count ? &roots[i + 1] : NULL}; }
    DependencySelection selection = {environment->view, scope->selection->owner, roots, baseline->count};
    unsigned char *needed = source_arena_allocate(arena, environment->view->count);
    if (!needed) { return SOURCE_PERMISSION_MEMORY; } memset(needed, 0, environment->view->count);
    const char *version = scope->version->logical_path; size_t version_length = strlen(version);
    for (const DeliverySourceDefinition *definition = copy->expansion.analysis->original.response.definitions; definition; definition = definition->next) {
        const SourceCloneMapping *source = clone(map, definition->identity); if (!source) { continue; }
        ModulePathShape from;
        if (!module_path_describe((const unsigned char *)source->identity, strlen(source->identity), &from)) { return SOURCE_PERMISSION_INVALID; }
        if (from.root != MODULE_ROOT_PROJECT || from.version_length != version_length || memcmp(source->identity, version, version_length)) { continue; }
        issue->source = source;
        size_t source_index = actual(environment->view, source->identity);
        if (!source_index) { return SOURCE_PERMISSION_MISSING; }
        const ModulePathValue *source_value = environment->view->entries[source_index - 1].identity;
        for (const DeliverySourceDependency *edge = definition->dependencies; edge; edge = edge->next) {
            issue->edge = edge; const SourceCloneMapping *target = clone(map, edge->target);
            issue->expected = target ? target->identity : edge->target;
            ModulePathShape to;
            if (!module_path_describe((const unsigned char *)issue->expected, strlen(issue->expected), &to)) { return SOURCE_PERMISSION_INVALID; }
            if (to.root == MODULE_ROOT_STANDARD) {
                const ModuleSource *present = NULL;
                if (module_catalog_find(environment->catalog, issue->expected, &present) != MODULE_OK) { return SOURCE_PERMISSION_MISSING; }
                continue;
            }
            size_t target_index = actual(environment->view, issue->expected);
            if (!target_index) { return SOURCE_PERMISSION_MISSING; }
            const ModulePathValue *target_value = environment->view->entries[target_index - 1].identity;
            issue->decision = dependency_edge_check(&selection, source_value, target_value);
            if (issue->decision == DEPENDENCY_EDGE_ALLOWED) { continue; }
            if (issue->decision != DEPENDENCY_EDGE_UNDECLARED) { return SOURCE_PERMISSION_DIRECTION; }
            if (copy->mode != SOURCE_COPY_EXTEND_PACKAGE) { return SOURCE_PERMISSION_UNDECLARED; }
            needed[target_index - 1] = 1;
        }
    }
    size_t count = 0;
    for (size_t i = 0; i < environment->view->count; i++) { if (needed[i]) { count++; } }
    if (count > SIZE_MAX / sizeof(DependencyViewEntry *)) { return SOURCE_PERMISSION_LIMIT; }
    const DependencyViewEntry **entries = count ? source_arena_allocate(arena, count * sizeof(*entries)) : NULL;
    if (count && !entries) { return SOURCE_PERMISSION_MEMORY; }
    size_t next = 0;
    for (size_t i = 0; i < environment->view->count; i++) { if (needed[i]) { entries[next++] = &environment->view->entries[i]; } }
    *output = (SourceSelectionRoots){entries, count}; *issue = (SourcePermissionIssue){0}; return SOURCE_PERMISSION_OK;
}
