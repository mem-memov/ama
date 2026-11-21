/**
 * @file
 * @brief Separate selector input preparation, storage and two exact DP kernels.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) fixes finite matching;
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) fixes alias scope.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns scratch;
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) bounds counts.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves sites.
 */
#ifndef AMA_SOURCE_CONFIGURATION_SELECTORS_INTERNAL_H
#define AMA_SOURCE_CONFIGURATION_SELECTORS_INTERNAL_H
#include "selector.h"

/** A finite stream of copied-base components followed by original mask components.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) never normalizes its suffix.
 */
typedef struct { const ModulePathValue *base; SourceSpan suffix; size_t index, offset; } SelectorCursor;
/** Finite component count and an upper bound on decoded scalar count.
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) sizes all arrays before allocation.
 */
typedef struct { size_t count, bytes; } SelectorMeasure;
/** Borrowed candidate component bytes; valid identity admission precedes preparation.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) excludes separators from word matches.
 */
typedef struct { const unsigned char *bytes; size_t length; } SelectorWord;
/** Operation-local DP storage, entirely retired by the match orchestrator.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) prevents retained scratch.
 */
typedef struct { SelectorWord *words; bool *path, *word; size_t word_capacity; } SelectorWorkspace;

/** Preserve original refusal without allocating under [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
DependencySelectorStatus selector_failure(SourceIssue *issue, SourceSpan span,
    DependencySelectorStatus status, const char *message, const char *expected);
/** Admit syntax and one package alias under [PKG-04](../../../../spec/project/packages.md#pkg-04). */
DependencySelectorStatus selector_input(SourceSpan expression, const ConfigurationAliasScope *package,
    SelectorCursor *output, SourceIssue *issue);
/** Consume one whole component under [PKG-05](../../../../spec/project/packages.md#pkg-05). */
bool selector_next(SelectorCursor *cursor, ModulePathPart *output);
/** Measure exact parts and byte-bounded scalars under [ARCH-05](../../../../spec/implementation/compiler.md#arch-05). */
DependencySelectorStatus selector_measure(SelectorCursor cursor, SourceSpan expression,
    SelectorMeasure *output, SourceIssue *issue);
/** Copy complete scalar storage under [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
DependencySelectorStatus selector_store(SourceArena *arena, SelectorCursor input, SelectorMeasure measure,
    SourceSpan expression, const DependencySelector **output, SourceIssue *issue);
/** Prepare checked local rows under [ARCH-05](../../../../spec/implementation/compiler.md#arch-05). */
DependencySelectorStatus selector_workspace(SourceArena *arena, const unsigned char *bytes, size_t length,
    size_t count, SourceSpan expression, SelectorWorkspace *output, SourceIssue *issue);
/** Exact scalar-prefix reachability under [PKG-05](../../../../spec/project/packages.md#pkg-05). */
bool selector_word_match(DependencySelectorPart pattern, SelectorWord word, bool *rows, size_t capacity);
#endif
