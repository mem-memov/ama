/** @file
 * @brief Select exact main/companion inputs before optional active subtree traversal.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) keeps topology-only
 * ancestors from pulling siblings or subtree dependencies into delivery.
 */
#include "internal.h"
#include <string.h>
SourceStatus acquisition_unit_capture(SourceArena *arena, SourceManager *texts, const char *parent,
    const char *basename, AcquisitionRole role, const AcquisitionUnitEntry **output, const char **failed_path) {
    if (!arena || !texts || !parent || !*parent || !basename || !*basename ||
        strchr(basename, '/') || strchr(basename, '.') || strchr(basename, '+') || !output || !failed_path ||
        (role != ACQUISITION_CONTEXT && role != ACQUISITION_ACTIVE)) { return SOURCE_INVALID_ARGUMENT; }
    AcquisitionUnitCapture capture = {.arena = arena, .texts = texts};
    module_catalog_init(&capture.entries, arena->allocator);
    AcquisitionUnitEntry *main = NULL;
    SourceStatus status = acquisition_unit_child(&capture, parent, basename, false, false, &main);
    const char **names = NULL; size_t count = 0;
    if (status == SOURCE_OK) {
        capture.failed = source_arena_copy(arena, parent);
        if (!capture.failed) { status = SOURCE_OUT_OF_MEMORY; }
        else {
            ModuleStatus listed = module_catalog_entries(&capture.entries, parent, &names, &count);
            if (listed != MODULE_OK) { status = listed == MODULE_OUT_OF_MEMORY ? SOURCE_OUT_OF_MEMORY : SOURCE_IO_ERROR; }
        }
    }
    size_t length = strlen(basename);
    for (size_t index = 0; status == SOURCE_OK && index < count; index++) {
        const char *name = names[index];
        if (strncmp(name, basename, length) || name[length] != '.' || !name[length + 1]) { continue; }
        AcquisitionUnitEntry *companion = NULL;
        status = acquisition_unit_child(&capture, parent, name, false, false, &companion);
    }
    if (status == SOURCE_OK && role == ACQUISITION_ACTIVE) {
        const char *tree = source_arena_join(arena, basename, "", "+");
        status = tree ? acquisition_unit_tree(&capture, parent, tree) : SOURCE_OUT_OF_MEMORY;
    }
    module_catalog_destroy(&capture.entries);
    if (status == SOURCE_OK) { *output = capture.first; *failed_path = NULL; }
    else { *failed_path = capture.failed; }
    return status;
}
