/** @file
 * @brief Assemble one immutable logical-version and physical-directory pair after route selection.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) keeps all modules of a
 * version tied to one physical version directory. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * keeps failed preparation under the same explicit arena owner.
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** Single allocation aligns the record and owns both complete terminated paths. */
typedef struct { AcquisitionLocation location; char bytes[]; } LocationStorage;
AcquisitionMapStatus acquisition_location(SourceArena *arena, const AcquisitionRoute *route,
    const AcquisitionLocation **output, SourceIssue *issue) {
    if (!issue) { return ACQUISITION_MAP_INVALID; }
    if (!arena || !route || !route->request || !route->mapping || !output ||
        route->prefix_length >= route->shape.version_length || route->shape.version_length > route->request->length) {
        return acquisition_map_failure(issue, (SourceSpan){0}, ACQUISITION_MAP_INVALID, "Invalid successful-route mapping handle");
    }
    SourceSpan origin = route->mapping->directory;
    const char *base = NULL; size_t base_length = 0;
    AcquisitionMapStatus status = acquisition_directory(arena, origin, &base, &base_length, issue);
    if (status != ACQUISITION_MAP_OK) { return status; }
    size_t start = route->prefix_length + 1;
    size_t suffix = route->shape.version_length - start;
    size_t separator = base_length != 0;
    if (!suffix || base_length > SIZE_MAX - suffix - separator) {
        return acquisition_map_failure(issue, origin, ACQUISITION_MAP_LIMIT, "Mapped directory length is not representable");
    }
    size_t directory_length = base_length + separator + suffix;
    size_t version_length = route->shape.version_length;
    size_t header = offsetof(LocationStorage, bytes);
    if (directory_length == SIZE_MAX || version_length == SIZE_MAX ||
        directory_length + 1 > SIZE_MAX - header || version_length + 1 > SIZE_MAX - header - directory_length - 1) {
        return acquisition_map_failure(issue, origin, ACQUISITION_MAP_LIMIT, "Mapped version storage is not representable");
    }
    size_t allocation = header + directory_length + 1 + version_length + 1;
    if (allocation < sizeof(LocationStorage)) { allocation = sizeof(LocationStorage); }
    LocationStorage *storage = source_arena_allocate(arena, allocation);
    if (!storage) { return acquisition_map_failure(issue, origin, ACQUISITION_MAP_OUT_OF_MEMORY, "Cannot allocate the mapped version paths"); }
    char *directory = storage->bytes;
    memcpy(directory, base, base_length);
    if (separator) { directory[base_length] = '/'; }
    memcpy(directory + base_length + separator, route->request->text + start, suffix); directory[directory_length] = 0;
    char *version = directory + directory_length + 1;
    memcpy(version, route->request->text, version_length); version[version_length] = 0;
    storage->location = (AcquisitionLocation){*route, version, directory, version_length, directory_length};
    *output = &storage->location;
    return ACQUISITION_MAP_OK;
}
