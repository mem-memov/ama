/** @file
 * @brief Separate option recognition, raw root/exclusion assembly and final request publication.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires complete explicit admission before any source operation starts.
 */
#ifndef AMA_PROJECT_SOURCE_REQUEST_INTERNAL_H
#define AMA_PROJECT_SOURCE_REQUEST_INTERNAL_H
#include "request.h"
/** Scalar kinds occupy the first six bits; appendable scope options never use
 * last-value semantics or share a scalar repeat bit. */
typedef enum { PROJECT_SOURCE_OPTION_PREVIEW, PROJECT_SOURCE_OPTION_APPLY,
    PROJECT_SOURCE_OPTION_COPY, PROJECT_SOURCE_OPTION_EXTEND,
    PROJECT_SOURCE_OPTION_REPLACE, PROJECT_SOURCE_OPTION_COMPILER,
    PROJECT_SOURCE_OPTION_ROOT, PROJECT_SOURCE_OPTION_STANDARD,
    PROJECT_SOURCE_OPTION_DESTINATION_ROOT, PROJECT_SOURCE_OPTION_DESTINATION_STANDARD,
    PROJECT_SOURCE_OPTION_EXCLUDE, PROJECT_SOURCE_OPTION_DESTINATION_EXCLUDE } ProjectSourceOptionKind;
/** One fixed token and its exact following argument arity; no --key=value sugar. */
typedef struct { const char *name; ProjectSourceOptionKind kind; size_t arity; } ProjectSourceOption;
/** Raw exclusion provenance, resolved only against the matching explicit list. */
typedef struct { const char *root, *path; size_t argument; bool destination; } ProjectSourceExclusion;
/** Stack-only orchestration over request-owned bounded scratch. */
typedef struct {
    ProjectSourceRequest *request;
    ProjectSourceExclusion *exclusions;
    size_t exclusion_count;
    unsigned seen;
} ProjectSourceRequestWork;
/** Recognize one exact option without effects or allocation. */
const ProjectSourceOption *project_source_option(const char *);
/** Reserve argv-bounded root/exclusion metadata, retaining every failed prefix. */
ProjectSourceRequestStatus project_source_request_reserve(ProjectSourceRequestWork *, size_t);
/** Consume validated nonempty values and enforce scalar/category consistency. */
ProjectSourceRequestStatus project_source_request_option(ProjectSourceRequestWork *, const ProjectSourceOption *, size_t);
/** Attach exact scoped exclusions after all root declarations are known. */
ProjectSourceRequestStatus project_source_request_exclusions(ProjectSourceRequestWork *);
/** Require complete scalar/root policy after every option has been consumed. */
ProjectSourceRequestStatus project_source_request_finish(ProjectSourceRequestWork *);
#endif
