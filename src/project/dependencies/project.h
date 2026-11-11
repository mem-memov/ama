#ifndef AMA_PROJECT_DEPENDENCIES_H
#define AMA_PROJECT_DEPENDENCIES_H

#include "ast/ast.h"
#include "diagnostics/adapters/adapters.h"

typedef enum {
    PROJECT_OK, PROJECT_CATALOG_ERROR, PROJECT_MISSING, PROJECT_DRAFT,
    PROJECT_SYNTAX_ERROR, PROJECT_OUT_OF_MEMORY, PROJECT_INVALID_ENTRY
} ProjectStatus;
typedef enum { PROJECT_MODULE_LOADING, PROJECT_MODULE_READY } ProjectModuleState;
typedef struct ProjectModule ProjectModule;
typedef struct ProjectDependency {
    struct ProjectDependency *next;
    SourceSpan requirement;
    const char *logical_path;
    ProjectModule *target;
} ProjectDependency;
struct ProjectModule {
    ProjectModule *next;
    const ModuleSource *source;
    AstModule syntax;
    ProjectModuleState state;
    ProjectModule *requested_by;
    SourceSpan requirement;
    ProjectDependency *first;
    ProjectDependency *last;
};
typedef struct {
    ModuleCatalog catalog;
    SourceArena arena;
    ProjectModule *first;
    ProjectModule *last;
    ProjectModule *entry;
    ProjectStatus status;
    Diagnostic error;
    DiagnosticRelated catalog_related;
    bool attempted;
    bool valid;
} Project;

/* Initialize, load once, inspect, destroy. Owns catalog, ASTs, graph and errors.
 * Roots/entry are borrowed only during load. Entry must be a valid logical path. Only a valid project publishes a
 * usable closure; partial records remain owned on failure for error reporting.
 * Discovery accepts module cycles, not a proof of layout/initialization legality.
 * The filesystem must remain stable during catalog construction. */
void project_init(Project *project, SourceAllocator allocator);
void project_destroy(Project *project);
ProjectStatus project_load(Project *project, const ModuleRoot *roots,
    size_t root_count, const char *entry);
const ProjectModule *project_find(const Project *project, const char *logical_path);

#endif
