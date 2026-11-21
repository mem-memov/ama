/**
 * @file
 * @brief Reduce logical path components while retaining original source provenance.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) defines exact
 * stack reduction; [PKG-04](../../../../spec/project/packages.md#pkg-04) supplies
 * already normalized alias targets. [CONST-05](../../../../spec/language/declarations.md#const-05)
 * selects a constant only afterwards. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * gives every result/scratch allocation one caller-supplied arena owner.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) separately
 * validates known canonical metadata and complete module endpoints.
 */
#ifndef AMA_SOURCE_PATH_NORMALIZE_H
#define AMA_SOURCE_PATH_NORMALIZE_H
#include "source/path/root.h"
#include "source/memory/arena.h"
#include "source/text/issue.h"

/** Refusals distinguish malformed input, missing scope, underflow, size and allocation limits.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) refuses root
 * escape rather than guessing a replacement base; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * keeps allocation refusal distinct from source validity.
 */
typedef enum {
    MODULE_PATH_OK,
    MODULE_PATH_INVALID,
    MODULE_PATH_MISSING_BASE,
    MODULE_PATH_UNKNOWN_ALIAS,
    MODULE_PATH_UNDERFLOW,
    MODULE_PATH_LIMIT,
    MODULE_PATH_OUT_OF_MEMORY
} ModulePathStatus;

/** One complete component with arena-owned bytes and optional original source interval.
 * Generated metadata bases have no invented origin. [SRC-06](../../../../spec/language/source-and-names.md#src-06)
 * preserves complete surviving components; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * requires the borrowed source owner to outlive provenance consumption.
 */
typedef struct {
    const unsigned char *bytes;
    size_t length;
    SourceSpan origin;
} ModulePathPart;

/** Immutable arena-owned stack normal form; source origins remain borrowed.
 * No dot/parent/alias marker survives in parts. A normal form is not necessarily
 * a complete module identity: a later suffix can remove intermediate components.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) checks complete
 * endpoint structure in the actual module-reference context, not prematurely.
 */
typedef struct {
    ModuleRootKind root;
    const ModulePathPart *parts;
    size_t count;
    const char *text;
    size_t length;
    SourceSpan expression;
} ModulePathValue;

/** Synchronous read-only lookup of a complete tilde-prefixed key.
 * Return NULL for unknown keys, otherwise a value produced by this component.
 * Returned data need survive this call only; original source owners outlive
 * the resulting provenance. [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * resolves scope before use and forbids a lazy recursive alias-text expansion.
 */
typedef const ModulePathValue *(*ModulePathAliasLookup)(void *context, SourceSpan key);

/** Selected prefix/member, both owning their data through the destination arena.
 * The source expression and component origins remain borrowed. [CONST-05](../../../../spec/language/declarations.md#const-05)
 * fixes exactly the final-component split after complete normalization.
 */
typedef struct {
    const ModulePathValue *module;
    ModulePathPart member;
    SourceSpan expression;
} ModuleConstantPath;

/** Normalize one complete raw path against explicit defining-directory/alias inputs.
 * The arena and writable output/issue are valid; base/lookup results were produced
 * by this component. Relative syntax requires base; absolute/alias forms ignore
 * it. Lookup occurs at most once and no filesystem callback exists. Every result
 * copies its bytes; only original SourceSpan provenance remains borrowed.
 * Output changes only on success, failure scratch remains arena-owned, and input
 * bytes never change. [SRC-06](../../../../spec/language/source-and-names.md#src-06)
 * supplies the terminating stack algorithm without an arbitrary depth limit.
 */
ModulePathStatus module_path_normalize(SourceArena *arena, SourceSpan expression,
    const ModulePathValue *base, ModulePathAliasLookup lookup, void *context,
    const ModulePathValue **output, SourceIssue *issue);

/** Copy known canonical catalog/version/base metadata without fabricating source positions.
 * Accept a valid canonical namespace prefix or complete module, including / and //.
 * Input bytes need survive only this call. Output publication/failure ownership
 * matches normalize. [SRC-01](../../../../spec/language/source-and-names.md#src-01)
 * validates logical structure independently of physical directory spelling.
 */
ModulePathStatus module_path_from_canonical(SourceArena *arena, const unsigned char *bytes,
    size_t length, const ModulePathValue **output, SourceIssue *issue);

/** Copy the containing directory of a complete module identity.
 * Reject a mere namespace/version/area prefix. No parent file is looked up or
 * imported. [SRC-06](../../../../spec/language/source-and-names.md#src-06) uses
 * this directory, never the module's own name, as its relative reference base.
 */
ModulePathStatus module_path_parent(SourceArena *arena, const ModulePathValue *module,
    const ModulePathValue **output, SourceIssue *issue);

/** Split the final component and require the preceding prefix to be a complete module.
 * No alternate prefix or implicit instance is attempted. Destination bytes are
 * independent of the input value's arena; source provenance remains borrowed.
 * Output changes only on success. [CONST-05](../../../../spec/language/declarations.md#const-05)
 * requires this operation after alias expansion and stack reduction.
 */
ModulePathStatus module_path_constant(SourceArena *arena, const ModulePathValue *path,
    ModuleConstantPath *output, SourceIssue *issue);
#endif
