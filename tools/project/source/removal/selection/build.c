/** @file
 * @brief Prepare complete original deletion sets from one shared admitted source view.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * includes actually present associated tests; [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * removes no unrelated dependency or topology-context unit.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"
/** Exact immutable view ownership prevents mixing unrelated source analyses. */
static bool admitted(const SourceTreeSelection *tree, const SourceAnalysis *analysis) {
    if (!tree || !tree->valid || !tree->members || !tree->capture || !tree->capture->valid || !analysis || !analysis->valid) { return false; }
    const DeliverySourceSnapshotView *capture = &tree->capture->view, *view = &analysis->view;
    return capture->entries == view->entries && capture->count == view->count
        && capture->versions == view->versions && capture->companions == view->companions;
}
/** Complete checked-export absence certifies a draft; no body text is guessed. */
static bool active(const SourceAnalysis *analysis, const char *identity) {
    for (const DeliverySourceDefinition *definition = analysis->original.response.definitions; definition; definition = definition->next) {
        if (!strcmp(definition->identity, identity)) { return true; }
    }
    return false;
}
/** The finite original catalog bounds physical expansion; graph edges are never traversed. */
static const SourceCapturedUnit *next_unit(const SourceRemovalSelection *selection) {
    for (const SourceCapturedUnit *unit = selection->tree->capture->units; unit; unit = unit->next) {
        const SourceRemovalProcessed *processed = selection->processed;
        while (processed && processed->unit != unit) { processed = processed->next; }
        if (!processed && source_removal_selection_contains_main(selection, unit)) { return unit; }
    }
    return NULL;
}
SourceRemovalSelectionStatus source_removal_selection_build(SourceRemovalSelection *selection,
    const SourceTreeSelection *tree, SourceAnalysis *analysis, const SourceDeclaredGroup *declared) {
    if (!selection || selection->attempted || !admitted(tree, analysis)) { return SOURCE_REMOVAL_SELECTION_INVALID; }
    selection->attempted = true;
    selection->tree = tree;
    selection->analysis = analysis;
    selection->domain_status = source_tree_selection_domain(tree, &selection->failed_path);
    if (selection->domain_status != SOURCE_TREE_DOMAIN_OK) { return selection->status = SOURCE_REMOVAL_SELECTION_DOMAIN; }
    if (!source_declared_groups_valid(tree->capture, declared)) { return selection->status = SOURCE_REMOVAL_SELECTION_INVALID; }
    if (!source_declared_groups_match_original(tree->capture, declared, &selection->failed_path)) {
        return selection->status = SOURCE_REMOVAL_SELECTION_CHANGED;
    }
    SourceRemovalGroup *raw = source_arena_allocate(&selection->arena, sizeof(*raw));
    if (!raw) { return selection->status = SOURCE_REMOVAL_SELECTION_MEMORY; }
    *raw = (SourceRemovalGroup){.files = tree->members};
    selection->groups = raw;
    const SourceCapturedUnit *unit;
    while ((unit = next_unit(selection))) {
        SourceRemovalProcessed *processed = source_arena_allocate(&selection->arena, sizeof(*processed));
        if (!processed) { return selection->status = SOURCE_REMOVAL_SELECTION_MEMORY; }
        *processed = (SourceRemovalProcessed){.next = selection->processed, .unit = unit};
        selection->processed = processed;
        selection->status = source_removal_selection_unit(selection, unit);
        if (selection->status != SOURCE_REMOVAL_SELECTION_OK) { return selection->status; }
        const char *identity = unit->source->logical_path;
        ModulePathShape shape;
        if (!module_path_describe((const unsigned char *)identity, strlen(identity), &shape)) {
            return selection->status = SOURCE_REMOVAL_SELECTION_INVALID;
        }
        if (shape.area == MODULE_PATH_SRC && active(analysis, identity)) {
            selection->status = source_removal_selection_tests(selection, unit);
            if (selection->status != SOURCE_REMOVAL_SELECTION_OK) { return selection->status; }
        }
        selection->status = source_removal_selection_declared(selection, declared, unit);
        if (selection->status != SOURCE_REMOVAL_SELECTION_OK) { return selection->status; }
    }
    selection->status = source_removal_selection_index(selection);
    if (selection->status == SOURCE_REMOVAL_SELECTION_OK) { selection->status = source_removal_selection_inputs(selection, declared); }
    selection->valid = selection->status == SOURCE_REMOVAL_SELECTION_OK;
    return selection->status;
}
