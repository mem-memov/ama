/** @file
 * @brief Reserve finite argv-bounded request metadata without fixed root or path-depth limits.
 * [ARCH-06](../../../../spec/implementation/compiler.md#arch-06)
 * requires exact capacity arguments and checked arithmetic, not heuristic limits.
 */
#include "internal.h"
#include <stdint.h>
ProjectSourceRequestStatus project_source_request_reserve(ProjectSourceRequestWork *work, size_t capacity) {
    if (capacity > SIZE_MAX / sizeof(ProjectSourceRootRequest) || capacity > SIZE_MAX / sizeof(ProjectSourceExclusion)) {
        return PROJECT_SOURCE_REQUEST_LIMIT;
    }
    work->request->roots = capacity ? source_arena_allocate(&work->request->arena, capacity * sizeof(*work->request->roots)) : NULL;
    work->request->destination_roots = capacity ? source_arena_allocate(&work->request->arena, capacity * sizeof(*work->request->destination_roots)) : NULL;
    work->exclusions = capacity ? source_arena_allocate(&work->request->arena, capacity * sizeof(*work->exclusions)) : NULL;
    return capacity && (!work->request->roots || !work->request->destination_roots || !work->exclusions)
        ? PROJECT_SOURCE_REQUEST_MEMORY : PROJECT_SOURCE_REQUEST_OK;
}
