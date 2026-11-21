/** @file
 * @brief Persist byte-exact selected test arguments and stage-specific outcomes.
 * [PROJECT-03](../../../spec/implementation/project-tools.md#project-03)
 * requires repeatable request records, including empty/newline-containing arguments.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "internal.h"
/** Every field is decimal byte length, colon, exact bytes, comma and newline.
 * Length-delimiting makes arbitrary argv bytes unambiguous without shell parsing. */
static bool field(FILE *file, const char *value) {
    size_t length = strlen(value);
    return fprintf(file, "%zu:", length) > 0 && fwrite(value, 1, length, file) == length && fputs(",\n", file) >= 0;
}
/** Allocate only a temporary joined path; return an exclusively created stream
 * owned by the caller. Path/allocation/open refusal never replaces a record. */
static FILE *open_record(const char *directory, const char *name, SourceAllocator allocator) {
    size_t left = strlen(directory), right = strlen(name);
    if (left > SIZE_MAX - right - 2) { return NULL; }
    char *path = allocator.allocate(allocator.context, left + right + 2);
    if (!path) { return NULL; }
    memcpy(path, directory, left); path[left] = '/'; memcpy(path + left + 1, name, right + 1);
    FILE *file = fopen(path, "wx");
    allocator.release(allocator.context, path);
    return file;
}
bool project_test_record_request(const ProjectTestJob *job, const char *directory, SourceAllocator allocator) {
    FILE *file = open_record(directory, "request.record", allocator);
    if (!file) { return false; }
    bool complete = field(file, "ama.test.request.v1") && field(file, job->name) &&
        field(file, job->module) && field(file, job->provenance) && fprintf(file, "%zu\n", job->argument_count) > 0;
    for (size_t index = 0; complete && index < job->argument_count; index++) { complete = field(file, job->arguments[index]); }
    int closed = fclose(file);
    return complete && closed == 0;
}
bool project_test_record_outcome(const ProjectTestOutcome *outcome, const char *directory, SourceAllocator allocator) {
    FILE *file = open_record(directory, "outcome.txt", allocator);
    if (!file) { return false; }
    bool complete = fprintf(file, "stage=%d\nexit=%d\nsignal=%d\n", (int)outcome->stage, outcome->exit_code, outcome->signal) > 0;
    int closed = fclose(file);
    return complete && closed == 0;
}
