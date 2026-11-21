/** @file
 * @brief Report original alias/configuration failures through the ordinary requester chain.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original
 * sites; [PKG-03](../../../../spec/project/packages.md#pkg-03) keeps INI text owned.
 */
#include "internal.h"

/** Forward an original issue without changing its location to normalized synthetic text.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves first
 * cause and [PKG-03](../../../../spec/project/packages.md#pkg-03) its source lifetime.
 */
bool project_configuration_failure(Project *project, ProjectModule *module, SourceIssue issue) {
    Diagnostic cause = {.code = DIAGNOSTIC_SYNTAX, .message = issue.message,
        .primary = {.span = issue.span}};
    if (issue.span.source == NULL) { cause.primary.path = source_path(module->source->text); }
    return project_report(project, PROJECT_INVALID_CONFIGURATION, module->requested_by,
        module->requirement, module->source->logical_path, &cause);
}
