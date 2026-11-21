/**
 * @file
 * @brief Preserve source-attributed alias-scope refusals without allocating diagnostics.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains original
 * sites; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) distinguishes
 * allocation refusal without publishing partially resolved targets.
 */
#include "internal.h"

ConfigurationScopeStatus configuration_scope_failure(SourceIssue *issue, SourceSpan span,
    ConfigurationScopeStatus status, const char *message, const char *expected) {
    *issue = (SourceIssue){span, message, expected};
    return status;
}

ConfigurationScopeStatus configuration_scope_path_status(ModulePathStatus status) {
    if (status == MODULE_PATH_OK) { return CONFIGURATION_SCOPE_OK; }
    if (status == MODULE_PATH_LIMIT) { return CONFIGURATION_SCOPE_LIMIT; }
    if (status == MODULE_PATH_OUT_OF_MEMORY) { return CONFIGURATION_SCOPE_OUT_OF_MEMORY; }
    return CONFIGURATION_SCOPE_INVALID;
}
