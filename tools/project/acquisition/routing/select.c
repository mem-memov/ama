/** @file
 * @brief Choose the maximum matching component prefix from exactly one immutable scope.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) forbids character-prefix
 * matches, target-scope borrowing and availability-dependent fallback.
 */
#include <string.h>
#include "route.h"
/** Preserve an original bounded request diagnostic without allocating on failure. */
static AcquisitionRouteStatus failure(SourceSpan request, SourceIssue *issue,
    AcquisitionRouteStatus status, const char *message, const char *expected) {
    if (issue) { *issue = (SourceIssue){source_span_valid(request) ? request : (SourceSpan){0}, message, expected}; }
    return status;
}
AcquisitionRouteStatus acquisition_route_select(const ConfigurationDocument *scope, const ModulePathValue *request,
    AcquisitionRoute *output, SourceIssue *issue) {
    ModulePathShape shape;
    if (!scope || !scope->valid || scope->status != CONFIGURATION_OK || scope->profile != CONFIGURATION_PACKAGE ||
        !output || !issue || !request ||
        !module_path_describe((const unsigned char *)request->text, request->length, &shape) ||
        shape.root != MODULE_ROOT_PROJECT || (shape.extent != MODULE_PATH_VERSION && shape.extent != MODULE_PATH_MODULE)) {
        return failure(request ? request->expression : (SourceSpan){0}, issue, ACQUISITION_ROUTE_INVALID, "Invalid repository request or requesting configuration",
            "one complete canonical ordinary version/module and one valid package-profile document");
    }
    const unsigned char *bytes = (const unsigned char *)request->text;
    size_t length = request->length;
    const ConfigurationRepository *selected = NULL;
    size_t longest = 0;
    for (const ConfigurationRepository *entry = scope->repositories; entry; entry = entry->next) {
        SourceSpan prefix = entry->prefix;
        size_t size = prefix.end - prefix.start;
        if (size >= length || bytes[size] != '/' || memcmp(bytes, source_bytes(prefix.source) + prefix.start, size)) { continue; }
        /* Matching full-component prefixes of one path form an inclusion chain.
         * Its longest byte prefix therefore also has the most components;
         * this implements PKG-06's component order exactly. */
        if (size > longest) { selected = entry; longest = size; }
    }
    if (!selected) {
        return failure(request->expression, issue, ACQUISITION_ROUTE_MISSING, "No repository mapping covers the requested identity",
            "an explicit mapping in the requesting configuration or initial acquisition environment");
    }
    *output = (AcquisitionRoute){scope, selected, request, shape, longest};
    return ACQUISITION_ROUTE_OK;
}
