/** @file
 * @brief Join source-unit, version and ancestor facts before one active expansion.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) retains context without
 * expanding its siblings, then promotes it only for a real active requirement.
 * [PKG-06](../../../../../spec/project/packages.md#pkg-06) fixes snapshot ownership.
 */
#include "internal.h"
#include <string.h>
AcquisitionClosureStatus acquisition_closure_structure(AcquisitionClosure *closure,
    const AcquisitionRoleRecord *record, const AcquisitionExpansion *fact) {
    if (!fact->module || !fact->module->logical_path || strcmp(fact->module->logical_path, record->identity) ||
        !fact->files || (fact->submodule_count && !fact->submodules)) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Incomplete selected module expansion");
    }
    const ModuleVersion *version = fact->module->version;
    bool standard = !strncmp(record->identity, "//", 2);
    if ((standard && (version || fact->source || fact->configuration || fact->guidance)) || (!standard && !version)) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Module namespace and selected version facts disagree");
    }
    if (version && (!fact->source || !fact->source->version || strcmp(version->logical_path, fact->source->version) ||
        !fact->configuration || fact->configuration->directory || !fact->configuration->file.content)) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Ordinary module expansion lacks its bound version/configuration");
    }
    AcquisitionClosureStatus status = acquisition_closure_files(closure, fact->files, record, fact->source, ACQUISITION_UNIT_MEMBER);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    status = acquisition_closure_files(closure, fact->configuration, record, fact->source, ACQUISITION_VERSION_CONFIGURATION);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    status = acquisition_closure_files(closure, fact->guidance, record, fact->source, ACQUISITION_VERSION_GUIDANCE);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    for (const AcquisitionContextFact *context = fact->contexts; context; context = context->next) {
        size_t length = context->identity ? strlen(context->identity) : 0;
        if (!length || strncmp(record->identity, context->identity, length) || record->identity[length] != '/' || !context->files) {
            return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Topology fact is not a retained module ancestor");
        }
        const AcquisitionRoleRecord *parent = NULL;
        status = acquisition_closure_request(closure, context->identity, ACQUISITION_CONTEXT, ACQUISITION_TOPOLOGY, record, &parent);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
        status = acquisition_closure_files(closure, context->files, parent, fact->source, ACQUISITION_UNIT_MEMBER);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
    }
    size_t prefix = strlen(record->identity);
    for (size_t index = 0; index < fact->submodule_count; index++) {
        const char *identity = fact->submodules[index];
        if (!identity || strncmp(identity, record->identity, prefix) || identity[prefix] != '/') {
            return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Structural descendant has a foreign logical identity");
        }
        const AcquisitionRoleRecord *child = NULL;
        status = acquisition_closure_request(closure, identity, ACQUISITION_ACTIVE, ACQUISITION_SUBMODULE, record, &child);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
    }
    return ACQUISITION_CLOSURE_OK;
}
