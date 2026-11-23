/** @file
 * @brief Bind copied version-selector mappings to complete raw-carried configuration origins.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * maps original version permissions through actual whole-version placement, never
 * an unrelated caller-proposed version with a convenient name.
 */
#include "internal.h"
#include <string.h>
/** Validate owning view membership before reading a supplied version descriptor. */
static bool member(const DependencyView *view, const DependencyViewEntry *entry) {
    for (size_t i = 0; i < view->count; i++) { if (&view->entries[i] == entry) { return true; } }
    return false;
}
/** Actual complete selected version identities are unique within one environment. */
static const SourcePackageScope *scope(const SourcePackageEnvironment *environment, const char *name) {
    const SourcePackageScope *row = environment->first;
    while (row && strcmp(row->version->logical_path, name)) { row = row->next; }
    return row;
}
bool source_operation_configuration_versions(const SourceOperationConfigurationWriteInputs *inputs) {
    for (size_t i = 0; i < inputs->version_count; i++) {
        const SourceSelectionVersionPair *pair = &inputs->versions[i];
        if (!member(inputs->original->view, pair->original) || !member(inputs->preview->view, pair->final)
            || pair->original->shape.extent != MODULE_PATH_VERSION || pair->final->shape.extent != MODULE_PATH_VERSION) { return false; }
        const SourcePackageScope *before = scope(inputs->original, pair->original->identity->text);
        const SourcePackageScope *after = scope(inputs->preview, pair->final->identity->text);
        if (!before || !after || !source_operation_configuration_carried(inputs, before, after)) { return false; }
    }
    return true;
}
