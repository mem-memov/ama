/**
 * @file
 * @brief Decide exact structural-unit and associated-test coverage from sparse selected roots.
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) retains full units without
 * activating context ancestors. [PKG-08](../../../../spec/project/packages.md#pkg-08)
 * requires actual source modules before a version's test directory is associated.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) does not assert target existence.
 */
#include "internal.h"

/** Whole-version selection associates only tests of its actual source records.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) does not invent an orphan source owner.
 */
static bool version_tests(const DependencyView *view, DependencyViewEntry target) {
    for (size_t index = 0; index < view->count; index++) {
        if (selection_associated(view->entries[index], target)) { return true; }
    }
    return false;
}

bool dependency_selection_covers(const DependencySelection *selection, const ModulePathValue *target) {
    ModulePathShape shape;
    if (target == NULL || !module_path_describe((const unsigned char *)target->text, target->length, &shape)
        || shape.root != MODULE_ROOT_PROJECT
        || (shape.extent != MODULE_PATH_VERSION && shape.extent != MODULE_PATH_MODULE)) { return false; }
    const DependencyViewEntry *actual = selection_find(selection->view, target->text, target->length);
    if (actual == NULL) { return false; }
    DependencyViewEntry candidate = *actual;
    for (const DependencySelectionRoot *root = selection->roots; root != NULL; root = root->next) {
        const DependencyViewEntry *entry = root->entry;
        if (entry->shape.extent == MODULE_PATH_VERSION) {
            if (selection_same_version(*entry, candidate)
                && (shape.extent == MODULE_PATH_VERSION || shape.area == MODULE_PATH_SRC
                    || (shape.area == MODULE_PATH_TESTS && version_tests(selection->view, candidate)))) { return true; }
        } else if (selection_prefix(entry->identity->text, entry->identity->length, target->text, target->length, false)
            || selection_associated(*entry, candidate)) { return true; }
    }
    return false;
}
