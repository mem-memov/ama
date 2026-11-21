/**
 * @file
 * @brief Reject duplicate endpoints and modules without an actual version record.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) limits matching to
 * eligible existing identities, including explicit empty versions.
 */
#include "internal.h"
DependencySelectionStatus selection_validate(const DependencyView *view, SourceIssue *issue) {
    for (size_t index = 0; index < view->count; index++) {
        const DependencyViewEntry *entry = &view->entries[index];
        const ModulePathValue *value = entry->identity;
        if (index != 0) {
            const ModulePathValue *previous = view->entries[index - 1].identity;
            if (selection_compare(previous->text, previous->length, value->text, value->length) == 0) {
                return selection_failure(issue, value->expression, DEPENDENCY_SELECTION_INVALID,
                    "Duplicate prepared dependency identity", "one authoritative record per complete identity");
            }
        }
        if (entry->shape.extent == MODULE_PATH_MODULE
            && selection_find(view, value->text, entry->shape.version_length) == NULL) {
            return selection_failure(issue, value->expression, DEPENDENCY_SELECTION_INVALID,
                "Prepared module has no version record", "its actual selected package-version identity");
        }
    }
    return DEPENDENCY_SELECTION_OK;
}
