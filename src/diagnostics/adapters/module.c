/**
 * @file
 * @brief Adapt explicit source-discovery failures into stable diagnostic records.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * Keep both conflicting paths and the exact malformed byte without allocating an error message.
 */
#include "adapters.h"

/** Immutable per-status explanation; no allocation is needed even for out-of-memory diagnostics. */
typedef struct {
    DiagnosticCode code;
    const char *message;
    const char *cause;
    const char *expected;
    const char *help;
} ModuleExplanation;

/** Exact indexed source-status mapping; MODULE_OK is rejected before indexing. */
static const ModuleExplanation explanations[] = {
    {DIAGNOSTIC_INVALID_PATH, NULL, NULL, NULL, NULL},
    {DIAGNOSTIC_MISSING_MODULE, "Module source was not found", "The exact path does not exist",
        "an extensionless module source", "Provide the module under a configured source root"},
    {DIAGNOSTIC_OUT_OF_MEMORY, "Source discovery could not allocate storage",
        "Allocation failed or a storage size is unrepresentable", NULL, NULL},
    {DIAGNOSTIC_IO_FAILURE, "Source input could not be read", "A filesystem operation failed",
        NULL, "Check the input path, accessibility and filesystem state"},
    {DIAGNOSTIC_INVALID_PATH, "Invalid module path", "The path does not satisfy the source-path contract",
        "dot-separated Unicode identifiers", NULL},
    {DIAGNOSTIC_UNSUPPORTED_PATH, "Unsupported filesystem path",
        "The path cannot be used with exact, regular-file source lookup", NULL,
        "Use exact UTF-8 entry spelling and regular files/directories without symlinks"},
    {DIAGNOSTIC_ORPHAN_DIRECTORY, "Submodule directory has no regular parent module",
        "The required sibling extensionless module is absent or not a regular file",
        "a sibling module file without the directory's final plus",
        "Create the parent module file beside its submodule directory"},
    {DIAGNOSTIC_AMBIGUOUS_MODULE, "More than one source defines the same module",
        "Configured roots contain conflicting logical identities", NULL,
        "Keep one source for this logical module identity"},
    {DIAGNOSTIC_FILE_ALIAS, "Source paths alias the same filesystem object",
        "Distinct source paths have equal device and inode identities", NULL,
        "Remove overlapping roots or replace aliases with distinct source files"},
    {DIAGNOSTIC_INVALID_UTF8, "Source input is not valid UTF-8",
        "A byte sequence is malformed, truncated, overlong or not a Unicode scalar",
        "well-formed UTF-8", "Save this source or ABI companion as valid UTF-8"}
};

bool diagnostic_from_module(const ModuleIssue *issue,
    DiagnosticRelated *related, Diagnostic *output) {
    if (issue == NULL || output == NULL || issue->status <= MODULE_OK
        || issue->status > MODULE_INVALID_UTF8
        || (issue->related_path != NULL && related == NULL)) {
        return false;
    }
    ModuleExplanation explanation = explanations[issue->status];
    Diagnostic diagnostic = {explanation.code, explanation.message, NULL,
        {{0}, issue->path}, explanation.cause, explanation.expected, explanation.help, NULL, 0};
    if (issue->source != NULL) {
        if (issue->offset >= source_length(issue->source)) {
            return false;
        }
        diagnostic.primary.span = (SourceSpan){issue->source, issue->offset, issue->offset + 1};
    }
    DiagnosticRelated note = {0};
    if (issue->related_path != NULL) {
        note = (DiagnosticRelated){{{0}, issue->related_path}, "conflicting source", NULL};
        diagnostic.related = &note;
        diagnostic.related_count = 1;
    }
    if (!diagnostic_valid(&diagnostic)) {
        return false;
    }
    if (diagnostic.related_count != 0) {
        *related = note;
        diagnostic.related = related;
    }
    *output = diagnostic;
    return true;
}
