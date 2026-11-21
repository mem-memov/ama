/**
 * @file
 * @brief Declare required-module registry, discovery and requester-diagnostic phases.
 *
 * [DEP-02](../../../spec/implementation/compiler.md#dep-02)
 * [PKG-04](../../../spec/project/packages.md#pkg-04) retains definition-owned scopes.
 * [PKG-03](../../../spec/project/packages.md#pkg-03) preserves configuration locations.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) preserves first causes.
 * Insert a loading record once before parsing its dependencies, so cycles reuse exact module identity.
 */
#ifndef AMA_PROJECT_DEPENDENCIES_INTERNAL_H
#define AMA_PROJECT_DEPENDENCIES_INTERNAL_H

#include "project.h"

/** Find an exact existing loading/ready record for internal closure construction; partial-project lookup is intentional here only.
 * Component contract: [DEP-02](../../../spec/implementation/compiler.md#dep-02). */
ProjectModule *project_lookup(Project *project, const char *path);
/** Reuse an existing identity or insert one loading record before parsing dependencies.
 * @param[out] output Changes only on success and borrows project lifetime. requester/requirement record first-discovery context.
 * @param[in,out] project Mutable project owner with registry/source storage valid through every output consumer.
 * @param[in] requester First requesting module, or null for the explicit entry.
 * @param[in] path Borrowed complete logical module path copied into project-owned storage as needed.
 * @param[in] requirement Original source span locating this request; its owner remains project-owned.
 * Component contract: [DEP-02](../../../spec/implementation/compiler.md#dep-02). */
bool project_require(Project *project, ProjectModule *requester,
    const char *path, SourceSpan requirement, ProjectModule **output);
/** Prepare the required definition's own module/package alias scopes before dependency discovery.
 * Captured companions are already loaded; outputs remain arena-owned on failure.
 * [PKG-04](../../../spec/project/packages.md#pkg-04) forbids caller/local inheritance.
 */
bool project_prepare_aliases(Project *, ProjectModule *);
/** Preserve original configuration/path refusal and requester context without invented source spans.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) locates the first cause;
 * [PKG-03](../../../spec/project/packages.md#pkg-03) retains original INI locations.
 */
bool project_configuration_failure(Project *, ProjectModule *, SourceIssue);
/** Parse a registered module once, reject required drafts/invalid entry, discover its dependencies, then mark ready. Failure preserves owned AST/context.
 * Component contract: [DEP-02](../../../spec/implementation/compiler.md#dep-02). */
bool project_parse_required(Project *project, ProjectModule *module);
/** Collect all represented AST path dependencies using finite explicit blocks; no C recursive module/body traversal.
 * Component contract: [DEP-02](../../../spec/implementation/compiler.md#dep-02). */
bool project_discover(Project *project, ProjectModule *module);
/** Validate retained source roles only after every registered target has parsed.
 * Failure retains the exact occurrence and requester chain without publishing a closure.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) distinguishes producers and Types.
 */
bool project_validate_kinds(Project *);
/** Copy the complete path, require its exact module and append one source-mapped edge. Partial allocations remain project-owned on failure.
 * Component contract: [DEP-02](../../../spec/implementation/compiler.md#dep-02). */
bool project_add_dependency(Project *project, ProjectModule *module, SourceSpan path, ProjectPathRole role);
/** Add one fixed language-required module identity while retaining the real source construct that requires it. Component contract: [DEP-02](../../../spec/implementation/compiler.md#dep-02). */
bool project_add_implicit_dependency(Project *, ProjectModule *, const char *, SourceSpan);
/** Copy entry spellings and merge original cause notes with the finite first-requester chain.
 * @return false, with all diagnostic text/related storage valid through project destruction; allocation failure replaces it with an allocation-free error.
 * Component contract: [DEP-02](../../../spec/implementation/compiler.md#dep-02). */
bool project_report(Project *project, ProjectStatus status, ProjectModule *requester,
    SourceSpan requirement, const char *path, const Diagnostic *cause);
/** Set an explicit out-of-memory status and immutable diagnostic message without further allocation.
 * Component contract: [DEP-02](../../../spec/implementation/compiler.md#dep-02). */
bool project_memory_failure(Project *project);

#endif
