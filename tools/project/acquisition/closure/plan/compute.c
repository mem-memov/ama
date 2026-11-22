/** @file
 * @brief Reach the least active source/test fixed point using complete prepared facts.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) bounds expansion to
 * one claim per active identity; [PROJECT-01](../../../../../spec/implementation/project-tools.md#project-01)
 * separates this controller from transport, compiler state and source mutation.
 */
#include "internal.h"
#include <string.h>
#include "source/path/logical.h"
AcquisitionClosureStatus acquisition_closure_compute(AcquisitionClosure *closure, const char *const *seeds, size_t count,
    const AcquisitionClosureProvider *provider) {
    if (!closure || closure->attempted) { return ACQUISITION_CLOSURE_INVALID; }
    closure->attempted = true;
    if ((count && !seeds) || !provider || !provider->expand || !provider->admit_test) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Incomplete delivery closure request");
    }
    for (size_t index = 0; index < count; index++) {
        const AcquisitionRoleRecord *record = NULL;
        AcquisitionClosureStatus status = acquisition_closure_request(closure, seeds[index], ACQUISITION_ACTIVE,
            ACQUISITION_SELECTED, NULL, &record);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
    }
    const AcquisitionRoleRecord *record;
    while ((record = acquisition_roles_take(&closure->roles))) {
        AcquisitionExpansion fact = {0}; SourceIssue issue = {0};
        AcquisitionClosureStatus status = provider->expand(provider->context, record->identity, &fact, &issue);
        if (status != ACQUISITION_CLOSURE_OK) {
            closure->issue = issue;
            return acquisition_closure_fail(closure, status, "Prepared source expansion failed");
        }
        for (const AcquisitionDeclaredInputs *inputs = fact.declared_inputs; inputs; inputs = inputs->next) {
            status = acquisition_closure_files(closure, inputs->files, record, inputs->source, ACQUISITION_DECLARED_INPUT);
            if (status != ACQUISITION_CLOSURE_OK) { return status; }
        }
        ModulePathShape shape;
        bool described = module_path_describe((const unsigned char *)record->identity, strlen(record->identity), &shape);
        if (!described) { return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Invalid retained delivery identity"); }
        if (shape.extent == MODULE_PATH_VERSION) {
            status = acquisition_closure_version(closure, record, &fact);
            if (status != ACQUISITION_CLOSURE_OK) { return status; }
            status = acquisition_closure_declared_modules(closure, record, fact.declared_modules);
            if (status != ACQUISITION_CLOSURE_OK) { return status; }
            continue;
        }
        status = acquisition_closure_structure(closure, record, &fact);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
        status = acquisition_closure_declared_modules(closure, record, fact.declared_modules);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
        const DeliverySourceDefinition *definition = NULL;
        status = acquisition_closure_definition(closure, record, &fact, &definition);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
        if (!definition) { continue; }
        status = acquisition_closure_tests(closure, record, &fact, provider);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
        AcquisitionRoleStatus expanded = acquisition_dependencies_expand(&closure->roles, record, definition,
            provider->follow_dependency, provider->context);
        if (expanded != ACQUISITION_ROLE_OK) {
            return acquisition_closure_fail(closure, expanded == ACQUISITION_ROLE_OUT_OF_MEMORY ? ACQUISITION_CLOSURE_OUT_OF_MEMORY
                : ACQUISITION_CLOSURE_INVALID, "Resolved delivery dependency expansion failed");
        }
    }
    closure->valid = true;
    return ACQUISITION_CLOSURE_OK;
}
