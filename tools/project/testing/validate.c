/** @file
 * @brief Validate complete test selections and physically disjoint output storage.
 * [PROJECT-03](../../../spec/implementation/project-tools.md#project-03)
 * keeps reports outside source namespaces and prevents conflicting job identity.
 */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "internal.h"
#include "source/path/logical.h"
/** Prefix comparison ends at a component boundary, never a textual sibling. */
static bool within(const char *path, const char *root) {
    size_t length = strlen(root);
    if (length == 1 && root[0] == '/') { return path[0] == '/'; }
    return !strncmp(path, root, length) && (!path[length] || path[length] == '/');
}
/** A missing source root has no current namespace; other failures are refusals. */
static bool outside(const char *output, const char *project, const char *area) {
    size_t prefix = strlen(project), tail = strlen(area);
    if (prefix > SIZE_MAX - tail - 2) { return false; }
    char *name = malloc(prefix + tail + 2);
    if (!name) { return false; }
    memcpy(name, project, prefix); name[prefix] = '/'; memcpy(name + prefix + 1, area, tail + 1);
    errno = 0;
    char *physical = realpath(name, NULL);
    int error = errno; free(name);
    if (!physical) { return error == ENOENT; }
    bool allowed = !within(output, physical);
    free(physical); return allowed;
}
bool project_tests_validate(const ProjectTestRequest *request, char **physical_parent) {
    if (!request || !request->project_root || !request->output_parent || !request->execute ||
        (request->count && !request->jobs) || (request->source_root_count && !request->source_roots)) { return false; }
    for (size_t index = 0; index < request->count; index++) {
        const ProjectTestJob *job = &request->jobs[index];
        if (!job->name || !*job->name || !job->module || !job->provenance ||
            !module_path_canonical((const unsigned char *)job->module, strlen(job->module)) ||
            (job->argument_count && !job->arguments)) { return false; }
        for (size_t arg = 0; arg < job->argument_count; arg++) { if (!job->arguments[arg]) { return false; } }
        for (size_t earlier = 0; earlier < index; earlier++) {
            if (!strcmp(job->name, request->jobs[earlier].name)) { return false; }
        }
    }
    char *project = realpath(request->project_root, NULL);
    char *output = realpath(request->output_parent, NULL);
    struct stat metadata;
    bool valid = project && output && stat(project, &metadata) == 0 && S_ISDIR(metadata.st_mode) &&
        stat(output, &metadata) == 0 && S_ISDIR(metadata.st_mode) &&
        outside(output, project, "packages") && outside(output, project, "dependencies");
    for (size_t index = 0; valid && index < request->source_root_count; index++) {
        const char *name = request->source_roots[index];
        char *source = name && *name ? realpath(name, NULL) : NULL;
        valid = source && stat(source, &metadata) == 0 && S_ISDIR(metadata.st_mode) && !within(output, source);
        free(source);
    }
    free(project);
    if (!valid) { free(output); return false; }
    *physical_parent = output;
    return true;
}
