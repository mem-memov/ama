/** @file
 * @brief Admit only owned original requesters and explicit usable declared input observations.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * joins one exact original domain; [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * cannot safely review a removal using foreign owners or unavailable input captures.
 */
#include <string.h>
#include "declared.h"
/** Membership is pointer-exact, so foreign metadata is never dereferenced. */
static bool owned(const SourceRootCapture *capture, const SourceCapturedUnit *requester) {
    for (const SourceCapturedUnit *unit = capture->units; unit; unit = unit->next) {
        if (unit == requester) { return true; }
    }
    return false;
}
bool source_declared_groups_valid(const SourceRootCapture *capture, const SourceDeclaredGroup *groups) {
    if (!capture || !capture->valid) { return false; }
    for (const SourceDeclaredGroup *group = groups; group; group = group->next) {
        if (!owned(capture, group->requester) || !group->files) { return false; }
        if (group->version) {
            const ModuleVersion *version = capture->catalog.versions;
            while (version && version != group->version) { version = version->next; }
            if (!version) { return false; }
        }
        for (const AcquisitionUnitEntry *file = group->files; file; file = file->next) {
            if (!file->file.path || file->file.path[0] != '/' || file->file.state != DELIVERY_FILE_PRESENT
                || file->file.issue != PATH_FOUND
                || (file->directory ? file->file.content != NULL : file->file.content == NULL)) { return false; }
            if (group->version) {
                size_t length = strlen(group->version->physical_path);
                if (strncmp(file->file.path, group->version->physical_path, length)
                    || (file->file.path[length] && file->file.path[length] != '/')) { return false; }
            }
        }
    }
    return true;
}
