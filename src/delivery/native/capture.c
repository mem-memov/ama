/** @file
 * @brief Associate exact instruction bytes and optional guidance with native origins.
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01) forbids
 * filling selected-copy holes; [BUILD-05](../../../spec/implementation/application-builds.md#build-05)
 * makes unavailable guidance report data rather than a compilation prerequisite.
 */
#include <string.h>
#include "capture.h"

/** Allocate exact physical parent/basename projections without interpreting any
 * logical path syntax under [BUILD-01](../../../spec/implementation/application-builds.md#build-01).
 */
static SourceStatus instruction(SourceArena *arena, SourceManager *texts,
    const ModuleSource *source, DeliveryFileSnapshot *output) {
    char *parent = source_arena_copy(arena, source_path(source->text));
    if (parent == NULL) { return SOURCE_OUT_OF_MEMORY; }
    char *slash = strrchr(parent, '/');
    const char *basename = slash == NULL ? parent : slash + 1;
    const char *name = source_arena_join(arena, basename, "", ".cmake");
    if (name == NULL) { return SOURCE_OUT_OF_MEMORY; }
    if (slash != NULL) { *slash = 0; }
    return delivery_file_capture(arena, texts,
        slash == NULL ? "." : (*parent == 0 ? "/" : parent), name, output);
}

/** Share only the identical selected version owner; no name-based fallback or
 * README search is permitted under [BUILD-05](../../../spec/implementation/application-builds.md#build-05).
 */
static SourceStatus guidance(SourceArena *arena, SourceManager *texts,
    const DeliveryNativeCapture *first, DeliveryNativeCapture *row) {
    const ModuleVersion *version = row->origin->source->version;
    if (version == NULL) { return SOURCE_OK; }
    for (const DeliveryNativeCapture *prior = first; prior != NULL; prior = prior->next) {
        if (prior->origin->source->version == version) { row->guidance = prior->guidance; return SOURCE_OK; }
    }
    DeliveryFileSnapshot *snapshot = source_arena_allocate(arena, sizeof(*snapshot));
    if (snapshot == NULL) { return SOURCE_OUT_OF_MEMORY; }
    SourceStatus status = delivery_file_capture(arena, texts, version->physical_path, "README.md", snapshot);
    if (status == SOURCE_OK) { row->guidance = snapshot; }
    return status;
}

bool delivery_native_capture(SourceArena *arena, SourceManager *texts, const DeliveryNativeOrigin *origins,
    const DeliveryNativeCapture **output, Diagnostic *error) {
    if (arena == NULL || texts == NULL || output == NULL || error == NULL) { return false; }
    DeliveryNativeCapture *first = NULL, **tail = &first;
    for (const DeliveryNativeOrigin *origin = origins; origin != NULL; origin = origin->next) {
        DeliveryNativeCapture *row = source_arena_allocate(arena, sizeof(*row));
        SourceStatus status = SOURCE_OUT_OF_MEMORY;
        if (row != NULL) {
            *row = (DeliveryNativeCapture){.origin = origin};
            status = instruction(arena, texts, origin->source, &row->cmake);
            if (status == SOURCE_OK && row->cmake.state == DELIVERY_FILE_UNAVAILABLE) {
                *error = (Diagnostic){.code = DIAGNOSTIC_IO_FAILURE,
                    .message = "Selected native CMake companion is unavailable", .primary = {.path = row->cmake.path}};
                return false;
            }
            if (status == SOURCE_OK) { status = guidance(arena, texts, first, row); }
            if (status == SOURCE_OK) {
                const char *failed = NULL;
                status = delivery_native_resources(arena, texts, origin->source, &row->resources, &failed);
                if (status == SOURCE_IO_ERROR) {
                    *error = (Diagnostic){.code = DIAGNOSTIC_IO_FAILURE,
                        .message = "Selected native resource is unavailable", .primary = {.path = failed}};
                    return false;
                }
            }
        }
        if (status != SOURCE_OK) {
            *error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
                .message = "Native build-input capture could not retain a snapshot"};
            return false;
        }
        *tail = row; tail = &row->next;
    }
    *output = first;
    return true;
}
