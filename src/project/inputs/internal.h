/**
 * @file
 * @brief Separate input admission, closure capture and exact invalidation operations.
 * [PKG-03](../../../spec/project/packages.md#pkg-03) retains local companion observations.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) keeps graph identity and original input ownership explicit.
 */
#ifndef AMA_PROJECT_INPUTS_INTERNAL_H
#define AMA_PROJECT_INPUTS_INTERNAL_H
#include "inputs.h"
/** Snapshot-owned edge to a live node in the same graph. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
typedef struct ProjectInputEdge {
    struct ProjectInputEdge *next;
    ProjectInput *target;
} ProjectInputEdge;
/** Arena-owned exact key and edges; content is immutable and private-source-owned or borrowed from the captured Project. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
struct ProjectInput {
    ProjectInput *next;
    ProjectInputs *owner;
    ProjectInputKind kind;
    const char *key;
    const SourceText *content;
    ProjectInputEdge *first;
    bool changed;
};
/** Publish an immutable source view without copying its bytes; source must outlive graph. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
SourceStatus project_input_source(ProjectInputs *, ProjectInputKind, const char *, const SourceText *, ProjectInput **);
/** Return one physical file node, reusing retained Ama/ABI text or loading a native file only on first admission. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
SourceStatus project_input_file(ProjectInputs *, const SourceText *, const char *, ProjectInput **);
/** Encode the complete ordered root/configuration inventory into one owned configuration node. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
SourceStatus project_input_roots(ProjectInputs *, const ModuleRoot *, size_t, ProjectInput **);
/** Record one activated module's exact optional companion presence and bytes.
 * One presence byte distinguishes absent from present-empty configuration; no
 * physical input is reopened. Scratch is released on every return. [PKG-03](../../../spec/project/packages.md#pkg-03)
 * fixes the local file and [DEP-03](../../../spec/implementation/compiler.md#dep-03)
 * invalidates only its actual consumers. Source remains Project-borrowed.
 */
SourceStatus project_input_aliases(ProjectInputs *, const ModuleSource *, ProjectInput *);
/** Attach one module's exact source/companion files and resolved module dependencies to its consumer node. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
SourceStatus project_input_module(ProjectInputs *, const ProjectModule *, ProjectInput *);
#endif
