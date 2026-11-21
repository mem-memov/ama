/** @file
 * @brief Keep descriptor and staging ownership private to empty-project initialization.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * separates preparation, publication and operation-owned cleanup.
 */
#ifndef AMA_PROJECT_INIT_INTERNAL_H
#define AMA_PROJECT_INIT_INTERNAL_H
#include <sys/stat.h>
#include <limits.h>
#include "init.h"
#include "content.h"
/** One operation owns all descriptors and allocated bytes; name borrows path. */
typedef struct {
    SourceAllocator allocator;
    ProjectInitControl control;
    ProjectInitResult result;
    char *path;
    const char *parent_path;
    const char *name;
    char stage_name[sizeof(long) * CHAR_BIT + sizeof(size_t) * CHAR_BIT + 16];
    char *stage_path;
    int parent, stage, packages;
    bool staged;
} ProjectInitState;
/** Record only the first failure, returning false for direct propagation. */
bool project_init_fail(ProjectInitState *, ProjectInitStatus, int, const char *);
/** Admit one explicit operation or retain cancellation; no filesystem effects. */
bool project_init_permit(ProjectInitState *, ProjectInitOperation, const char *);
/** Open physical parent and obtain exclusive lease; never follow symlinks. */
bool project_init_parent(ProjectInitState *, const char *);
/** Require absent/empty ordinary destination without changing it. */
bool project_init_preflight(ProjectInitState *);
/** Prepare the fixed complete scaffold in a private sibling directory. */
bool project_init_prepare(ProjectInitState *);
/** Recheck destination and atomically publish the prepared directory. */
bool project_init_publish(ProjectInitState *);
/** Release owned descriptors/storage; retain/report a stage that cannot be removed. */
void project_init_cleanup(ProjectInitState *);
#endif
