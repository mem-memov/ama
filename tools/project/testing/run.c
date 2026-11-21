/** @file
 * @brief Execute prepared test jobs with separate fresh directories and no shared runtime state.
 * [PROJECT-03](../../../spec/implementation/project-tools.md#project-03)
 * separates selection from execution; [BUILD-03](../../../spec/implementation/application-builds.md#build-03)
 * supplies the later accepted compilation adapter rather than an invented test ABI.
 */
#define _XOPEN_SOURCE 700
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "internal.h"
ProjectTestResult project_tests(const ProjectTestRequest *request, SourceAllocator allocator, ProjectTestOutcome *outcomes) {
    ProjectTestResult result = {.message = "Invalid test selection or output namespace"};
    char *parent = NULL;
    if (!allocator.allocate || !allocator.release || !request || (request->count && !outcomes) ||
        !project_tests_validate(request, &parent)) { return result; }
    size_t length = strlen(parent);
    if (length > SIZE_MAX - 32) { free(parent); return result; }
    result.directory = allocator.allocate(allocator.context, length + 32);
    if (!result.directory) { free(parent); result.message = "Cannot retain the experiment directory"; return result; }
    (void)sprintf(result.directory, "%s/ama-tests-XXXXXX", parent); free(parent);
    if (!mkdtemp(result.directory)) {
        allocator.release(allocator.context, result.directory); result.directory = NULL;
        result.message = "Cannot create an exclusive experiment directory"; return result;
    }
    length = strlen(result.directory);
    const size_t suffix = sizeof(size_t) * CHAR_BIT + 2;
    if (length > SIZE_MAX - suffix) { result.message = "Experiment path exceeds addressable storage"; return result; }
    char *job_directory = allocator.allocate(allocator.context, length + suffix);
    if (!job_directory) { result.message = "Cannot retain a test job path"; return result; }
    result.success = true; result.message = NULL;
    for (size_t index = 0; index < request->count; index++) {
        (void)sprintf(job_directory, "%s/%zu", result.directory, index);
        ProjectTestOutcome outcome = {.stage = PROJECT_TEST_PREPARATION, .exit_code = 1};
        bool recorded = mkdir(job_directory, 0700) == 0 &&
            project_test_record_request(&request->jobs[index], job_directory, allocator) &&
            request->execute(request->context, &request->jobs[index], job_directory, &outcome) &&
            outcome.stage >= PROJECT_TEST_PREPARATION && outcome.stage <= PROJECT_TEST_COMPLETE &&
            outcome.exit_code >= 0 && outcome.exit_code <= 255 && outcome.signal >= 0 &&
            (!outcome.signal || !outcome.exit_code) &&
            project_test_record_outcome(&outcome, job_directory, allocator);
        if (!recorded) { result.success = false; result.message = "Test infrastructure failed; retain the experiment directory"; break; }
        outcomes[index] = outcome; result.completed++;
        if (outcome.stage != PROJECT_TEST_COMPLETE || outcome.exit_code || outcome.signal) { result.success = false; }
    }
    allocator.release(allocator.context, job_directory);
    return result;
}
