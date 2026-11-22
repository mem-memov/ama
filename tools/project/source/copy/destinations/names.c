/** @file
 * @brief Name active sources before projecting tests through their final source associations.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves explicit placements and complete additional suffixes. It assigns
 * tests by deepest mapped association independently of role discovery order.
 */
#include "internal.h"
#include "source/path/logical.h"
#include <stdint.h>
#include <string.h>
/** Keep all explicit names exactly; derive only additional ordinary source names. */
static SourceCopyDestinationsStatus sources(SourceCopyDestinations *result, SourceCopyDestinationWork *work) {
    for (size_t i = 0; i < result->count; i++) {
        result->failed_role = work->entries[i].role;
        work->entries[i].version = work->assignments[i].version;
        const SourceCloneMapping *fixed = source_copy_destination_explicit(result, work->entries[i].source);
        if (fixed) {
            work->entries[i].identity = source_arena_copy(&result->arena, fixed->identity);
            if (!work->entries[i].identity) { return SOURCE_COPY_DESTINATIONS_MEMORY; }
        } else {
            ModulePathShape shape;
            if (!module_path_describe((const unsigned char *)work->entries[i].source->logical_path,
                strlen(work->entries[i].source->logical_path), &shape)) { return SOURCE_COPY_DESTINATIONS_NAMES; }
            if (shape.area == MODULE_PATH_SRC) {
                SourceStatus mapped = source_additional_module_identity(&result->arena,
                    &work->assignments[i], &work->entries[i].identity);
                if (mapped != SOURCE_OK) {
                    return mapped == SOURCE_OUT_OF_MEMORY ? SOURCE_COPY_DESTINATIONS_MEMORY : SOURCE_COPY_DESTINATIONS_NAMES;
                }
            }
        }
    }
    return SOURCE_COPY_DESTINATIONS_OK;
}
/** Derive both sides of every actual source association through shared PKG-08 logic. */
static SourceCopyDestinationsStatus associations(SourceCopyDestinations *result, SourceCopyDestinationWork *work) {
    for (size_t i = 0; i < result->count; i++) { work->test_count += work->associations[i] != NULL; }
    if (!work->test_count) { return SOURCE_COPY_DESTINATIONS_OK; }
    if (work->test_count > SIZE_MAX / sizeof(*work->tests)) { return SOURCE_COPY_DESTINATIONS_MEMORY; }
    work->tests = source_arena_allocate(&result->arena, work->test_count * sizeof(*work->tests));
    if (!work->tests) { return SOURCE_COPY_DESTINATIONS_MEMORY; }
    size_t count = 0;
    for (size_t i = 0; i < result->count; i++) {
        if (!work->associations[i]) { continue; }
        result->failed_role = work->entries[i].role;
        const char *destination = NULL;
        SourceStatus status = acquisition_test_namespace(&result->arena, work->entries[i].identity, &destination);
        if (status != SOURCE_OK) {
            return status == SOURCE_OUT_OF_MEMORY ? SOURCE_COPY_DESTINATIONS_MEMORY : SOURCE_COPY_DESTINATIONS_NAMESPACE;
        }
        work->tests[count++] = (SourceTestDirectoryMap){work->associations[i], destination};
    }
    return SOURCE_COPY_DESTINATIONS_OK;
}
SourceCopyDestinationsStatus source_copy_destination_names(SourceCopyDestinations *result,
    SourceCopyDestinationWork *work) {
    SourceCopyDestinationsStatus status = sources(result, work);
    if (status != SOURCE_COPY_DESTINATIONS_OK) { return status; }
    status = associations(result, work);
    if (status != SOURCE_COPY_DESTINATIONS_OK) { return status; }
    for (size_t i = 0; i < result->count; i++) {
        if (work->entries[i].identity) { continue; }
        result->failed_role = work->entries[i].role;
        SourceTestMapStatus mapped = source_associated_test_identity(&result->arena, work->tests,
            work->test_count, work->entries[i].source->logical_path, &work->entries[i].identity);
        if (mapped == SOURCE_TEST_MAP_OUTSIDE) {
            SourceStatus named = source_additional_module_identity(&result->arena, &work->assignments[i],
                &work->entries[i].identity);
            if (named != SOURCE_OK) {
                return named == SOURCE_OUT_OF_MEMORY ? SOURCE_COPY_DESTINATIONS_MEMORY : SOURCE_COPY_DESTINATIONS_NAMES;
            }
        } else if (mapped != SOURCE_TEST_MAP_OK) {
            return mapped == SOURCE_TEST_MAP_MEMORY ? SOURCE_COPY_DESTINATIONS_MEMORY : SOURCE_COPY_DESTINATIONS_NAMES;
        }
    }
    return SOURCE_COPY_DESTINATIONS_OK;
}
