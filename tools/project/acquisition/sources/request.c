/** @file
 * @brief Gate acquisition on the prepared whole-version view before scoped remote planning.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) forbids remote hole
 * filling; [PKG-06](../../../../spec/project/packages.md#pkg-06) scopes every
 * remaining route and interns its exact transport key without invoking Git.
 */
#include <string.h>
#include "internal.h"
AcquisitionSourcesStatus acquisition_sources_request(AcquisitionSources *sources, const ConfigurationDocument *scope,
    const ModulePathValue *request, AcquisitionSourceRequest *output, AcquisitionSourcesIssue *issue) {
    ModulePathShape shape;
    if (!sources || !request || !output || !issue ||
        !module_path_describe((const unsigned char *)request->text, request->length, &shape) ||
        shape.root != MODULE_ROOT_PROJECT || (shape.extent != MODULE_PATH_VERSION && shape.extent != MODULE_PATH_MODULE)) {
        return acquisition_sources_failure(issue, request ? request->expression : (SourceSpan){0}, ACQUISITION_SOURCES_INVALID, "Invalid complete acquisition identity", NULL);
    }
    for (const ModulePackage *package = sources->prepared; package; package = package->next) {
        if (strlen(package->logical) == shape.version_length && !memcmp(package->logical, request->text, shape.version_length)) {
            *output = (AcquisitionSourceRequest){.local = package, .origin = request->expression};
            return ACQUISITION_SOURCES_OK;
        }
    }
    AcquisitionRoute route;
    SourceIssue cause;
    AcquisitionRouteStatus selected = acquisition_route_select(scope, request, &route, &cause);
    if (selected != ACQUISITION_ROUTE_OK) {
        *issue = (AcquisitionSourcesIssue){.issue = cause};
        return selected == ACQUISITION_ROUTE_MISSING ? ACQUISITION_SOURCES_MISSING : ACQUISITION_SOURCES_INVALID;
    }
    const AcquisitionLocation *location = NULL;
    AcquisitionMapStatus mapped = acquisition_location(&sources->arena, &route, &location, &cause);
    if (mapped != ACQUISITION_MAP_OK) {
        *issue = (AcquisitionSourcesIssue){.issue = cause};
        return mapped == ACQUISITION_MAP_OUT_OF_MEMORY ? ACQUISITION_SOURCES_OUT_OF_MEMORY :
            mapped == ACQUISITION_MAP_LIMIT ? ACQUISITION_SOURCES_LIMIT : ACQUISITION_SOURCES_INVALID;
    }
    const AcquisitionSnapshot *snapshot = NULL; bool created = false;
    AcquisitionSourcesStatus status = acquisition_snapshot_request(sources, &route, &snapshot, &created, issue);
    if (status != ACQUISITION_SOURCES_OK) { return status; }
    *output = (AcquisitionSourceRequest){.location = location, .snapshot = snapshot,
        .resolve_snapshot = created, .origin = request->expression};
    return ACQUISITION_SOURCES_OK;
}
