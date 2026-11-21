/** @file
 * @brief Separate read-only experiment validation, records and job execution.
 * [PROJECT-03](../../../spec/implementation/project-tools.md#project-03)
 * retains explicit input/result attribution at each process boundary.
 */
#ifndef AMA_PROJECT_TESTING_INTERNAL_H
#define AMA_PROJECT_TESTING_INTERNAL_H
#include "testing.h"
/** Validate borrowed selection and return malloc-owned physical output parent.
 * All validation is read-only; caller frees physical_parent. */
bool project_tests_validate(const ProjectTestRequest *, char **physical_parent);
/** Persist exact length-delimited request bytes before invoking an adapter. */
bool project_test_record_request(const ProjectTestJob *, const char *, SourceAllocator);
/** Persist the observed stage/status/signal after adapter completion. */
bool project_test_record_outcome(const ProjectTestOutcome *, const char *, SourceAllocator);
#endif
