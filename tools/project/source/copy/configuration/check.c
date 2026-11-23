/** @file
 * @brief Compare exact prepared root names with independently rebuilt package selections.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids an old wildcard gaining unintended roots; ordinary successful edge
 * checking cannot establish equality of complete dependency permissions.
 */
#include <string.h>
#include "check.h"
/** Scope ownership must be actual, not a byte-equal copied descriptor. */
static bool owns(const SourcePackageEnvironment *environment, const SourcePackageScope *scope) {
    if (!environment || !environment->valid || !scope) { return false; }
    for (const SourcePackageScope *row = environment->first; row; row = row->next) { if (row == scope) { return true; } }
    return false;
}
/** Check pointer membership before using an expected entry's identity. */
static bool member(const DependencyView *view, const DependencyViewEntry *entry) {
    for (size_t i = 0; i < view->count; i++) { if (&view->entries[i] == entry) { return true; } }
    return false;
}
SourceConfigurationCheckStatus source_copy_configuration_check(
    const SourcePackageEnvironment *before, const SourcePackageScope *prepared, const SourceSelectionRoots *expected,
    const SourcePackageEnvironment *after, const SourcePackageScope *actual, SourceConfigurationCheckIssue *issue) {
    if (!issue) { return SOURCE_CONFIGURATION_CHECK_INVALID; } *issue = (SourceConfigurationCheckIssue){0};
    if (!owns(before, prepared) || !owns(after, actual) || !expected || (expected->count && !expected->entries)) { return SOURCE_CONFIGURATION_CHECK_INVALID; }
    for (size_t i = 0; i < expected->count; i++) {
        if (!member(before->view, expected->entries[i])) { return SOURCE_CONFIGURATION_CHECK_INVALID; }
        for (size_t j = 0; j < i; j++) { if (expected->entries[j] == expected->entries[i]) { return SOURCE_CONFIGURATION_CHECK_INVALID; } }
    }
    if (strcmp(prepared->version->logical_path, actual->version->logical_path)
        || strcmp(prepared->version->physical_path, actual->version->physical_path)
        || strcmp(source_path(prepared->document.source), source_path(actual->document.source))) { return SOURCE_CONFIGURATION_CHECK_SCOPE; }
    for (size_t i = 0; i < expected->count; i++) {
        const DependencySelectionRoot *root = actual->selection->roots;
        while (root && strcmp(root->entry->identity->text, expected->entries[i]->identity->text)) { root = root->next; }
        if (!root) { issue->expected = expected->entries[i]; return SOURCE_CONFIGURATION_CHECK_MISSING; }
    }
    for (const DependencySelectionRoot *root = actual->selection->roots; root; root = root->next) {
        size_t i = 0;
        while (i < expected->count && strcmp(root->entry->identity->text, expected->entries[i]->identity->text)) { i++; }
        if (i == expected->count) { issue->actual = root->entry; return SOURCE_CONFIGURATION_CHECK_EXTRA; }
    }
    return SOURCE_CONFIGURATION_CHECK_OK;
}
