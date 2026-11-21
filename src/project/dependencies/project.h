/**
 * @file
 * @brief Own one required-source closure and the lifetimes of its catalog, ASTs and errors.
 *
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03)
 * [PKG-04](../../../spec/project/packages.md#pkg-04) scopes aliases at their definitions.
 * [CONST-05](../../../spec/language/declarations.md#const-05) retains normalized
 * qualified members beside their exact requesting spans.
 * Loading cycles are permitted; layout and initialization legality belong to separate semantic relations.
 */
#ifndef AMA_PROJECT_DEPENDENCIES_H
#define AMA_PROJECT_DEPENDENCIES_H

#include "ast/ast.h"
#include "source/configuration/aliases/aliases.h"
#include "source/configuration/selection/selection.h"
#include "diagnostics/adapters/adapters.h"

/** Required-closure statuses distinguish source catalog, draft, syntax, source-kind, allocation and entry failures. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
typedef enum {
    PROJECT_OK, PROJECT_CATALOG_ERROR, PROJECT_MISSING, PROJECT_DRAFT,
    PROJECT_SYNTAX_ERROR, PROJECT_OUT_OF_MEMORY, PROJECT_INVALID_ENTRY, PROJECT_INVALID_KIND,
    PROJECT_INVALID_CONFIGURATION
} ProjectStatus;
/** Registry admission state; loading records exist before parsing to terminate module cycles. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
typedef enum { PROJECT_MODULE_LOADING, PROJECT_MODULE_READY } ProjectModuleState;
/** Exact syntactic role of a retained source occurrence.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) separates ordinary
 * value constructors from graph producers and explicitly selected exports.
 */
typedef enum { PROJECT_ORDINARY_PATH, PROJECT_STATIC_PRODUCER, PROJECT_STATIC_SELECTOR,
    PROJECT_CONSTANT_PATH } ProjectPathRole;
/**
 * Forward name for one exact project-owned loaded module, permitting reference cycles without
 * recursive C type expansion.
 * Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
typedef struct ProjectModule ProjectModule;
/** Project-owned exact path edge and original request site; target shares the unique module record. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
typedef struct ProjectDependency {
    struct ProjectDependency *next;
    SourceSpan requirement;
    const char *logical_path;
    const ModulePathValue *resolved_path;
    /** Resolved namespace/member for a whole qualified operand, otherwise null.
     * [CONST-05](../../../spec/language/declarations.md#const-05) selects from the
     * normalized path; component bytes need not occur in the written operand.
     */
    const ModuleConstantPath *constant_path;
    ProjectModule *target;
    /** A fixed language dependency retains its requiring site, which is not a written module-path span. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
    bool implicit;
    /** Required source kind, checked after parsing the complete closure.
     * [TPL-12](../../../spec/language/static-results.md#tpl-12) forbids a graph as a Type.
     */
    ProjectPathRole role;
} ProjectDependency;
/** Project-owned AST plus first-discovery requester context and ordered dependencies; source bytes belong to the catalog. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
struct ProjectModule {
    ProjectModule *next;
    const ModuleSource *source;
    AstModule syntax;
    ProjectModuleState state;
    ProjectModule *requested_by;
    SourceSpan requirement;
    const ConfigurationAliasScope *aliases;
    const DependencySelection *dependencies;
    ProjectDependency *first;
    ProjectDependency *last;
};
/** Opaque cached scope owned by the project arena; [PKG-04](../../../spec/project/packages.md#pkg-04)
 * shares one version environment without borrowing caller or sibling-module scopes.
 */
typedef struct ProjectAliasPackage ProjectAliasPackage;
/** Closure owner of catalog bytes, module ASTs, registry/work records and diagnostic storage. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
typedef struct {
    ModuleCatalog catalog;
    SourceArena arena;
    ProjectModule *first;
    ProjectModule *last;
    ProjectModule *entry;
    ProjectAliasPackage *alias_packages;
    const DependencyView *dependency_view;
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
/** Initialize owned catalog/project arenas with explicit borrowed allocator authority.
 * Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
void project_init(Project *project, SourceAllocator allocator);
/** Destroy every complete/failed module AST before catalog bytes and project arena. All project/diagnostic views expire.
 * Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
void project_destroy(Project *project);
/** Load one executable entry closure once from stable configured roots; return an explicit status without publishing partial success.
 * Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
ProjectStatus project_load(Project *project, const ModuleRoot *roots,
    size_t root_count, const char *entry);
/** Load entry plus an explicit finite list of additionally required modules, sharing each registry record. Roots/path arrays are borrowed only during load.
 * Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
ProjectStatus project_load_required(Project *project, const ModuleRoot *roots,
    size_t root_count, const char *entry, const char *const *required, size_t required_count);
/** Load from this owner's already built immutable catalog. The caller may have installed
 * exact copied in-memory source edits before parsing; the ordinary parser/registry/dependency
 * pipeline is reused. No source/AST records may have been registered yet.
 * Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
ProjectStatus project_load_catalog(Project *, const char *entry, const char *const *required, size_t required_count);
/** Load a finite explicit set from this owner's complete catalog without an executable entry.
 * Empty and constants-only sets are valid analysis domains; ordinary parsing/dependency rules remain unchanged.
 * Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
ProjectStatus project_load_catalog_set(Project *, const char *const *required, size_t required_count);
/** Return an exact borrowed module only from a valid completed closure, otherwise null. No lookup normalizes or invents a module path.
 * Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
const ProjectModule *project_find(const Project *project, const char *logical_path);

#endif
