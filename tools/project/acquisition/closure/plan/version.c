/** @file
 * @brief Retain an explicitly selected whole version before expanding its source modules.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) retains configuration
 * and structural source inputs; [PKG-08](../../../../../spec/project/packages.md#pkg-08)
 * applies mandatory tests to actual active src modules, not invented empty roots.
 */
#include "internal.h"
#include <string.h>
AcquisitionClosureStatus acquisition_closure_version(AcquisitionClosure *closure, const AcquisitionRoleRecord *record,
    const AcquisitionExpansion *fact) {
    if (fact->module || !fact->source || !fact->source->version || strcmp(fact->source->version, record->identity) ||
        !fact->files || !fact->configuration || fact->configuration->directory || !fact->configuration->file.content || fact->contexts || fact->test_directory || fact->test_files ||
        (fact->submodule_count && !fact->submodules)) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Incomplete selected-version expansion");
    }
    AcquisitionClosureStatus status = acquisition_closure_files(closure, fact->files, record, fact->source, ACQUISITION_UNIT_MEMBER);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    status = acquisition_closure_files(closure, fact->configuration, record, fact->source, ACQUISITION_VERSION_CONFIGURATION);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    status = acquisition_closure_files(closure, fact->guidance, record, fact->source, ACQUISITION_VERSION_GUIDANCE);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    const char *prefix = source_arena_join(&closure->arena, record->identity, "/", "src/");
    if (!prefix) { return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_OUT_OF_MEMORY, "Version source-prefix allocation failed"); }
    for (size_t index = 0; index < fact->submodule_count; index++) {
        const char *identity = fact->submodules[index];
        if (!identity || strncmp(identity, prefix, strlen(prefix))) {
            return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Selected-version source member has a foreign identity");
        }
        const AcquisitionRoleRecord *module = NULL;
        status = acquisition_closure_request(closure, identity, ACQUISITION_ACTIVE, ACQUISITION_SUBMODULE, record, &module);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
    }
    return ACQUISITION_CLOSURE_OK;
}
