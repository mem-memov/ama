/**
 * @file
 * @brief Separate path admission, finite reduction, storage and endpoint selection.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) fixes raw roots
 * and component reductions; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * owns every unpublished scratch prefix until arena destruction.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains original
 * issue sites; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * checks count/byte arithmetic before preparing storage.
 */
#ifndef AMA_SOURCE_PATH_NORMALIZE_INTERNAL_H
#define AMA_SOURCE_PATH_NORMALIZE_INTERNAL_H
#include "normalize.h"

/** Bounded component sequence and optional original source location.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) permits metadata
 * bases without inventing a file position for their generated spelling.
 */
typedef struct {
    const unsigned char *bytes;
    size_t length;
    SourceSpan origin;
} ModulePathFragment;

/** Admitted raw mode and component suffix; an initial relative parent is not discarded.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) distinguishes
 * absolute/relative/alias starts before any lookup or reduction.
 */
typedef struct {
    ModulePathFragment fragment;
    SourceSpan alias;
    ModuleRootKind root;
    bool relative;
    bool aliased;
} ModulePathInput;

/** Preserve the original failure site and static explanation without allocating.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) keeps reporting
 * possible even when storage preparation itself fails.
 */
ModulePathStatus path_normalization_failure(SourceIssue *issue, SourceSpan span,
    ModulePathStatus status, const char *message, const char *expected);
/** Validate the entire raw spelling and separate only the initial root/alias.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) never uses a
 * declaration inventory to decide where a path/name ends.
 */
ModulePathStatus path_normalization_read(SourceSpan expression, ModulePathInput *output, SourceIssue *issue);
/** Count nonempty components of an admitted fragment; zero length yields zero.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) forbids empty
 * components, so the count is bounded by positive original byte widths.
 */
size_t path_fragment_count(ModulePathFragment fragment);
/** Reduce an admitted fragment into sufficient scratch storage containing a valid prefix.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) consumes every
 * component exactly once and refuses the first pop from an empty stack.
 */
ModulePathStatus path_normalization_reduce(ModulePathFragment fragment, ModulePathPart *parts,
    size_t *count, SourceIssue *issue);
/** Copy a complete normal form into one aligned arena record with component/data views.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns even failed
 * preparation; [SRC-06](../../../../spec/language/source-and-names.md#src-06)
 * preserves exact bytes/provenance without a complete-endpoint assumption.
 */
ModulePathStatus path_normalization_store(SourceArena *arena, ModuleRootKind root,
    const ModulePathPart *parts, size_t count, SourceSpan expression,
    const ModulePathValue **output, SourceIssue *issue);
/** Reserve enough stack for an existing prefix plus this admitted fragment.
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) checks count
 * addition/multiplication before allocation; zero capacity needs no block.
 */
ModulePathStatus path_normalization_stack(SourceArena *arena, size_t prefix_count,
    ModulePathFragment fragment, ModulePathPart **output, SourceIssue *issue);
#endif
