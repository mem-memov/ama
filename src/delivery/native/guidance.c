/** @file
 * @brief Project actual emitted Type versions onto optional diagnostic guidance.
 * [BUILD-05](../../../spec/implementation/application-builds.md#build-05)
 * reports ordinary and native package origins while keeping README outside build
 * prerequisites and native dependency instructions.
 */
#include <string.h>
#include "guidance.h"

bool delivery_version_guidance(SourceArena *arena, SourceManager *texts, const CompilerFrontend *frontend,
    const DeliveryNativeCapture *native, const DeliveryVersionGuidance **output, Diagnostic *error) {
    if (arena == NULL || texts == NULL || frontend == NULL || !frontend->valid || !frontend->plan_initialized || frontend->authoring || output == NULL || error == NULL) { return false; }
    DeliveryVersionGuidance *first = NULL;
    for (const PlannedType *plan = frontend->plan.first; plan != NULL; plan = plan->next) {
        const ModuleVersion *version = plan->type->module->source->version;
        if (version == NULL) { continue; }
        DeliveryVersionGuidance **position = &first;
        while (*position != NULL && strcmp((*position)->version->logical_path, version->logical_path) < 0) {
            position = &(*position)->next;
        }
        if (*position != NULL && (*position)->version == version) { continue; }
        DeliveryVersionGuidance *row = source_arena_allocate(arena, sizeof(*row));
        if (row == NULL) { goto allocation_failure; }
        *row = (DeliveryVersionGuidance){.version = version, .next = *position};
        for (const DeliveryNativeCapture *item = native; item != NULL; item = item->next) {
            if (item->origin->source->version == version) { row->readme = item->guidance; break; }
        }
        if (row->readme == NULL) {
            DeliveryFileSnapshot *snapshot = source_arena_allocate(arena, sizeof(*snapshot));
            if (snapshot == NULL || delivery_file_capture(arena, texts, version->physical_path, "README.md", snapshot) != SOURCE_OK) {
                goto allocation_failure;
            }
            row->readme = snapshot;
        }
        *position = row;
    }
    *output = first;
    return true;
allocation_failure:
    *error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY, .message = "Package guidance metadata allocation failed"};
    return false;
}
