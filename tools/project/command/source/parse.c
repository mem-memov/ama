/** @file
 * @brief Recognize positional source operands and exact option arities before request publication.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates request recognition from source/compiler effects;
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires explicit operation, mode and dependency policy.
 */
#include "internal.h"
#include <string.h>
/** Map only the three defined verbs; no prefix or initialization fallback exists. */
static bool command(const char *verb, SourceOperationKind *kind) {
    if (!strcmp(verb, "copy")) { *kind = SOURCE_OPERATION_COPY; }
    else if (!strcmp(verb, "delete")) { *kind = SOURCE_OPERATION_DELETE; }
    else if (!strcmp(verb, "move")) { *kind = SOURCE_OPERATION_MOVE; }
    else { return false; }
    return true;
}
ProjectSourceRequestStatus project_source_request_parse(ProjectSourceRequest *request, int count, const char *const *arguments) {
    if (!request || request->attempted) { return PROJECT_SOURCE_REQUEST_INVALID; }
    request->attempted = true; request->status = PROJECT_SOURCE_REQUEST_ARGUMENTS;
    if (count < 2 || !arguments) { return request->status; }
    request->argument_count = (size_t)count; request->arguments = arguments;
    for (size_t i = 0; i < (size_t)count; i++) {
        if (!arguments[i]) { request->failed_argument = i; return request->status; }
    }
    if (!*arguments[0]) { return request->status; }
    request->failed_argument = 1;
    if (!command(arguments[1], &request->kind)) { return request->status = PROJECT_SOURCE_REQUEST_COMMAND; }
    size_t first = request->kind == SOURCE_OPERATION_DELETE ? 3 : 4;
    if ((size_t)count < first) { request->failed_argument = (size_t)count; return request->status; }
    request->source = arguments[2]; request->destination = first == 4 ? arguments[3] : NULL;
    for (size_t i = 2; i < first; i++) {
        if (!*arguments[i]) { request->failed_argument = i; return request->status = PROJECT_SOURCE_REQUEST_EMPTY_VALUE; }
    }
    request->compiler = "ama"; request->dependencies = SOURCE_COPY_KEEP_EXTERNAL;
    ProjectSourceRequestWork work = {.request = request};
    request->status = project_source_request_reserve(&work, (size_t)count);
    if (request->status != PROJECT_SOURCE_REQUEST_OK) { return request->status; }
    for (size_t i = first; i < (size_t)count;) {
        request->failed_argument = i;
        const ProjectSourceOption *option = project_source_option(arguments[i]);
        if (!option) { return request->status = PROJECT_SOURCE_REQUEST_OPTION; }
        if (option->arity > (size_t)count - i - 1) { return request->status = PROJECT_SOURCE_REQUEST_MISSING_VALUE; }
        for (size_t value = 1; value <= option->arity; value++) {
            if (!*arguments[i + value]) { request->failed_argument = i + value; return request->status = PROJECT_SOURCE_REQUEST_EMPTY_VALUE; }
        }
        request->status = project_source_request_option(&work, option, i);
        if (request->status != PROJECT_SOURCE_REQUEST_OK) { return request->status; }
        i += option->arity + 1;
    }
    request->failed_argument = (size_t)count;
    request->status = project_source_request_finish(&work);
    request->valid = request->status == PROJECT_SOURCE_REQUEST_OK;
    if (request->valid) { request->failed_argument = 0; }
    return request->status;
}
