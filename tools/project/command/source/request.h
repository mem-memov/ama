/** @file
 * @brief Own explicit source-operation arguments without filesystem, compiler or namespace inference.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps request recognition outside compiler dispatch; [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires one explicit preview/apply mode and mutually exclusive dependency policy.
 */
#ifndef AMA_PROJECT_SOURCE_REQUEST_H
#define AMA_PROJECT_SOURCE_REQUEST_H
#include "source/module/catalog.h"
#include "../../source/operation/kind.h"
#include "../../source/copy/mode.h"
/** Recognition never interprets APPLY as PREVIEW; dispatch decides support. */
typedef enum { PROJECT_SOURCE_PREVIEW, PROJECT_SOURCE_APPLY } ProjectSourceRequestMode;
/** Preserve the original path argument index together with its explicit family.
 * Exclusion arrays are owned; their strings and path borrow the original argv.
 * These are raw descriptors, not normalized/verified physical roots. */
typedef struct { ModuleRoot value; size_t argument; } ProjectSourceRootRequest;
/** Exact first failing recognition/ownership boundary, independent of stderr text. */
typedef enum { PROJECT_SOURCE_REQUEST_OK, PROJECT_SOURCE_REQUEST_INVALID,
    PROJECT_SOURCE_REQUEST_COMMAND, PROJECT_SOURCE_REQUEST_ARGUMENTS,
    PROJECT_SOURCE_REQUEST_OPTION, PROJECT_SOURCE_REQUEST_MISSING_VALUE,
    PROJECT_SOURCE_REQUEST_EMPTY_VALUE, PROJECT_SOURCE_REQUEST_REPEATED,
    PROJECT_SOURCE_REQUEST_MODE, PROJECT_SOURCE_REQUEST_POLICY,
    PROJECT_SOURCE_REQUEST_SCOPE, PROJECT_SOURCE_REQUEST_ROOTS,
    PROJECT_SOURCE_REQUEST_EXCLUSION, PROJECT_SOURCE_REQUEST_LIMIT,
    PROJECT_SOURCE_REQUEST_MEMORY } ProjectSourceRequestStatus;
/** Single-attempt owner. Borrow all argv strings/array through destruction.
 * valid certifies only request syntax/accounting; source existence, root aliases,
 * canonical identities, cross-version permission requirements and all operation
 * proof/publication gates remain consumers' obligations. Never copy a live owner. */
typedef struct {
    SourceArena arena;
    const char *const *arguments;
    size_t argument_count, failed_argument;
    const char *source, *destination, *compiler;
    ProjectSourceRootRequest *roots, *destination_roots;
    size_t root_count, destination_root_count;
    SourceOperationKind kind;
    SourceCopyDependencyMode dependencies;
    ProjectSourceRequestMode mode;
    ProjectSourceRequestStatus status;
    bool replace, attempted, valid;
} ProjectSourceRequest;
/** Initialize empty scratch without reading environment or filesystem state. */
void project_source_request_init(ProjectSourceRequest *, SourceAllocator);
/** Release all failed/successful prefixes; no borrowed argument is freed. */
void project_source_request_destroy(ProjectSourceRequest *);
/** Parse copy/move SOURCE DESTINATION OPTIONS or delete SOURCE OPTIONS, including
 * argv[0]. Require exactly one --preview/--apply and at least one explicit
 * --root/--standard-root. Support --copy-dependencies, --extend-package,
 * --replace, --compiler FILE (default literal "ama" for later PATH lookup),
 * --destination-root/--destination-standard-root DIR and scoped exclusions:
 * --exclude ROOT RELATIVE and --destination-exclude ROOT RELATIVE. ROOT repeats
 * an exact raw root argument in its corresponding list, independently of option
 * order. No prefix guessing, path normalization or shell evaluation occurs.
 * Root options can repeat with distinct paths; equal path duplicates refuse.
 * All scalar repeats, unknown options, empty/missing values, conflicting modes/
 * policies and delete dependency/replacement/destination options refuse. Exact
 * exclusions use nonempty slash-separated physical components without empty,
 * '.' or '..' components; repeated exclusions refuse. Paths otherwise remain
 * physical byte strings, not Ama identifiers. Publish valid only after complete
 * admission; failed scratch remains owned. Repeated parsing leaves prior state.
 * No I/O, processes, network, mutations or compiler-state dependencies. For A
 * argv entries and B spelling bytes, O(A squared) key comparisons, O(A*B) byte
 * work and O(A)
 * metadata/pointer storage; checked representation refuses before overflow.
 */
ProjectSourceRequestStatus project_source_request_parse(ProjectSourceRequest *, int, const char *const *);
#endif
