/** @file
 * @brief Create one complete empty project through an explicit owned transaction.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * requires preflight, private staging, atomic publication and limited cleanup.
 */
#ifndef AMA_PROJECT_INIT_H
#define AMA_PROJECT_INIT_H
#include <stdbool.h>
#include "source/memory/allocator.h"
/** Observable operation boundaries; admission may cancel before an operation. */
typedef enum {
    PROJECT_INIT_PARENT, PROJECT_INIT_LEASE, PROJECT_INIT_PREFLIGHT,
    PROJECT_INIT_STAGE, PROJECT_INIT_DIRECTORY, PROJECT_INIT_FILE,
    PROJECT_INIT_PUBLISH, PROJECT_INIT_CLEANUP
} ProjectInitOperation;
/** Borrowed optional cancellation/progress authority; never called after return. */
typedef struct {
    void *context;
    bool (*permit)(void *, ProjectInitOperation, const char *);
} ProjectInitControl;
/** Primary operation result; cleanup failure is a separate observation. */
typedef enum {
    PROJECT_INIT_OK, PROJECT_INIT_INVALID, PROJECT_INIT_CONFLICT, PROJECT_INIT_BUSY,
    PROJECT_INIT_IO, PROJECT_INIT_MEMORY, PROJECT_INIT_CANCELLED
} ProjectInitStatus;
/** Primary failure is retained even if cleanup fails. retained_stage is allocated
 * with the supplied allocator only when manual recovery is necessary; caller
 * releases it with that allocator. Other strings have static lifetime. */
typedef struct {
    ProjectInitStatus status;
    int system_error;
    const char *message;
    bool cleanup_failed;
    char *retained_stage;
} ProjectInitResult;
/** Initialize a new/empty ordinary directory under an existing ordinary parent.
 * Paths/allocator/control are borrowed synchronously. No compiler/network/Git.
 * Failure before atomic publication preserves the destination. See
 * [transaction proof](../../../docs/implementation/project-tool.md). */
ProjectInitResult project_init(const char *, SourceAllocator, ProjectInitControl);
#endif
