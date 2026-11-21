/**
 * @file
 * @brief Prepare a shared immutable sorted view while borrowing stable normalized identities.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) requires actual version/module
 * records. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed
 * preparation; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) checks array bounds.
 */
#include <stddef.h>
#include "internal.h"
/** Flexible storage preserves alignment under [ARCH-05](../../../../spec/implementation/compiler.md#arch-05). */
typedef struct { DependencyView view; DependencyViewEntry entries[]; } ViewStorage;
DependencySelectionStatus dependency_view_build(SourceArena *arena,
    const DependencyViewInput *identities, size_t count, const DependencyView **output, SourceIssue *issue) {
    size_t header = offsetof(ViewStorage, entries);
    if (count > (SIZE_MAX - header) / sizeof(DependencyViewEntry)) {
        return selection_failure(issue, (SourceSpan){0}, DEPENDENCY_SELECTION_LIMIT,
            "Prepared dependency view size is not representable", "a representable identity count");
    }
    if (count != 0 && identities == NULL) {
        return selection_failure(issue, (SourceSpan){0}, DEPENDENCY_SELECTION_INVALID,
            "Missing prepared identity array", "a readable complete identity array");
    }
    size_t size = header + count * sizeof(DependencyViewEntry);
    if (size < sizeof(ViewStorage)) { size = sizeof(ViewStorage); }
    ViewStorage *storage = source_arena_allocate(arena, size);
    if (storage == NULL) {
        return selection_failure(issue, (SourceSpan){0}, DEPENDENCY_SELECTION_OUT_OF_MEMORY,
            "Prepared dependency view allocation failed", "available identity-view storage");
    }
    storage->view = (DependencyView){count == 0 ? NULL : storage->entries, count};
    for (size_t index = 0; index < count; index++) {
        const ModulePathValue *value = identities[index].identity;
        ModulePathShape shape;
        if (value == NULL || !module_path_describe((const unsigned char *)value->text, value->length, &shape)
            || shape.root != MODULE_ROOT_PROJECT
            || (shape.extent != MODULE_PATH_VERSION && shape.extent != MODULE_PATH_MODULE)) {
            return selection_failure(issue, value == NULL ? (SourceSpan){0} : value->expression,
                DEPENDENCY_SELECTION_INVALID, "Ineligible prepared dependency endpoint", "an actual ordinary version or module");
        }
        size_t prefix = identities[index].test_directory_prefix;
        if ((shape.area != MODULE_PATH_TESTS && prefix != 0)
            || (shape.area == MODULE_PATH_TESTS && prefix > shape.components - shape.version_components - 2)) {
            return selection_failure(issue, value->expression, DEPENDENCY_SELECTION_INVALID,
                "Invalid test-directory provenance", "the verified leading ordinary-directory count before the main test filename");
        }
        DependencyViewEntry entry = {value, shape, prefix};
        size_t cursor = index;
        while (cursor != 0 && selection_compare(storage->entries[cursor - 1].identity->text,
            storage->entries[cursor - 1].identity->length, value->text, value->length) > 0) {
            storage->entries[cursor] = storage->entries[cursor - 1]; cursor--;
        }
        storage->entries[cursor] = entry;
    }
    DependencySelectionStatus status = selection_validate(&storage->view, issue);
    if (status == DEPENDENCY_SELECTION_OK) { *output = &storage->view; }
    return status;
}
