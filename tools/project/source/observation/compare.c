/** @file
 * @brief Compare complete selected catalog observations with the retained original capture.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * fixes original bindings and selected versions; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects a newer body/configuration or module set rather than adopting it.
 */
#include "internal.h"
#include <string.h>
/** NULL preserves captured absence; otherwise exact keys and bytes must agree. */
static bool same_text(const SourceText *left, const SourceText *right) {
    return !left || !right ? left == right : !strcmp(source_path(left), source_path(right)) &&
        source_length(left) == source_length(right) && !memcmp(source_bytes(left), source_bytes(right), source_length(left));
}
/** Record one immutable original mismatch without producing a replacement view. */
static SourceObservationStatus changed(SourceObservedImage *observed, const char *path) {
    observed->failed_path = path; return observed->status = SOURCE_OBSERVATION_CHANGED;
}
SourceObservationStatus source_observation_compare(SourceObservedImage *observed) {
    const ModuleCatalog *original = &observed->original->catalog;
    for (const ModuleSource *source = observed->catalog.first; source; source = source->next) {
        const ModuleSource *old = NULL;
        if (module_catalog_find(original, source->logical_path, &old) != MODULE_OK ||
            old->device != source->device || old->inode != source->inode || !same_text(old->text, source->text) ||
            !same_text(old->configuration, source->configuration)) { return changed(observed, source_path(source->text)); }
    }
    for (const ModuleSource *old = original->first; old; old = old->next) {
        const ModuleSource *source = NULL;
        if (module_catalog_find(&observed->catalog, old->logical_path, &source) != MODULE_OK) { return changed(observed, source_path(old->text)); }
    }
    for (const ModuleVersion *version = observed->catalog.versions; version; version = version->next) {
        const ModuleVersion *old = original->versions;
        while (old && strcmp(old->logical_path, version->logical_path)) { old = old->next; }
        if (!old || strcmp(old->physical_path, version->physical_path) || !same_text(old->configuration, version->configuration)) {
            return changed(observed, version->physical_path);
        }
    }
    for (const ModuleVersion *old = original->versions; old; old = old->next) {
        const ModuleVersion *version = observed->catalog.versions;
        while (version && strcmp(old->logical_path, version->logical_path)) { version = version->next; }
        if (!version) { return changed(observed, old->physical_path); }
    }
    return SOURCE_OBSERVATION_OK;
}
