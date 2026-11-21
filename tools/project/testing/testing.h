/** @file
 * @brief Orchestrate independent explicitly selected test microprograms.
 * [PROJECT-03](../../../spec/implementation/project-tools.md#project-03)
 * requires separate output/state ownership and exact experiment records.
 */
#ifndef AMA_PROJECT_TESTING_H
#define AMA_PROJECT_TESTING_H
#include <stdbool.h>
#include <stddef.h>
#include "source/memory/allocator.h"
/** One explicit experiment; names/arguments/provenance borrow caller storage.
 * provenance records the prepared closure/configuration/source/tool identities;
 * the build adapter supplies its native/toolchain record. No code is discovered
 * or executed while selecting these inputs. */
typedef struct {
    const char *name;
    const char *module;
    const char *const *arguments;
    size_t argument_count;
    const char *provenance;
} ProjectTestJob;
/** Stage-specific observed result; unsuccessful tests do not stop independent jobs. */
typedef enum { PROJECT_TEST_PREPARATION, PROJECT_TEST_COMPILATION, PROJECT_TEST_EXECUTION, PROJECT_TEST_COMPLETE } ProjectTestStage;
/** Terminal observation of one requested stage, independent of success. */
typedef struct { ProjectTestStage stage; int exit_code; int signal; } ProjectTestOutcome;
/** Synchronous build/run adapter. It receives an exclusive newly created job
 * directory, owns no other job state and must write detailed logs/provenance
 * there. Return false only for an infrastructure failure that prevents an
 * observed outcome; ordinary compiler/program refusal is a recorded outcome.
 * T242 connects the CMake adapter after its acceptance. */
typedef bool (*ProjectTestExecute)(void *, const ProjectTestJob *, const char *, ProjectTestOutcome *);
/** Borrowed prepared selection; output_parent must exist outside packages/ and
 * dependencies/ source namespaces of project_root, including physical aliases. */
typedef struct {
    const char *project_root;
    const char *output_parent;
    const ProjectTestJob *jobs;
    size_t count;
    void *context;
    ProjectTestExecute execute;
    /** Additional prepared source namespaces, including every selected Standard
     * root or extra compiler root. Caller supplies the complete resolved set. */
    const char *const *source_roots;
    size_t source_root_count;
} ProjectTestRequest;
/** Results never own source or execution state. directory, when nonnull, is an
 * allocator-owned path retained even after partial failure; caller releases the
 * string and controls report-directory retention. completed counts recorded jobs. */
typedef struct {
    bool success;
    size_t completed;
    char *directory;
    const char *message;
} ProjectTestResult;
/** Validate the whole selection before creating output. Run sequential isolated
 * jobs (no shared mutable contexts), recording exact requests and outcomes.
 * outcomes has request.count writable elements; invalid plans leave it untouched. */
ProjectTestResult project_tests(const ProjectTestRequest *, SourceAllocator, ProjectTestOutcome *outcomes);
#endif
