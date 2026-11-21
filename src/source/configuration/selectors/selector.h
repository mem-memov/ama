/**
 * @file
 * @brief Compile immutable dependency patterns and decide exact eligible identity matches.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) defines scalar and
 * component-chain stars. [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * permits only package aliases; [SRC-06](../../../../spec/language/source-and-names.md#src-06)
 * supplies already normalized real bases. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * separates compiled ownership and local matching scratch; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * bounds storage. [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * retains original issue sites.
 */
#ifndef AMA_SOURCE_CONFIGURATION_SELECTORS_SELECTOR_H
#define AMA_SOURCE_CONFIGURATION_SELECTORS_SELECTOR_H
#include <stdint.h>
#include "source/configuration/aliases/aliases.h"

/** Resource failures remain distinct from invalid spelling/scope.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) never publishes
 * partially compiled patterns or a match after resource refusal.
 */
typedef enum { DEPENDENCY_SELECTOR_OK, DEPENDENCY_SELECTOR_INVALID,
    DEPENDENCY_SELECTOR_LIMIT, DEPENDENCY_SELECTOR_OUT_OF_MEMORY } DependencySelectorStatus;

/** One owned scalar sequence and its original component provenance.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) makes exactly one star
 * a nonempty component-chain token; all other patterns stay within one component.
 */
typedef struct {
    const uint32_t *scalars;
    size_t count;
    SourceSpan origin;
    bool chain;
} DependencySelectorPart;

/** Arena-owned scalar/component data with borrowed SourceText provenance only.
 * No parsed document or normalized alias target is retained.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) expands aliases once
 * before [PKG-05](../../../../spec/project/packages.md#pkg-05) matches masks.
 */
typedef struct {
    const DependencySelectorPart *parts;
    size_t count;
    SourceSpan expression;
} DependencySelector;

/** Compile an original selector with an optional immutable package scope.
 * Arena/output/issue are valid. A supplied scope must be a package scope.
 * Scalars are copied; only original SourceText owners outlive this result.
 * Failure leaves output unchanged and scratch owned by arena. No I/O occurs.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) never cancels wildcard
 * components or accepts a standard-root selector.
 */
DependencySelectorStatus dependency_selector_compile(SourceArena *arena, SourceSpan expression,
    const ConfigurationAliasScope *package, const DependencySelector **output, SourceIssue *issue);

/** Match one bounded identity using operation-local allocator scratch.
 * Selector is a successful compiled result, bytes are readable for length,
 * output/issue and allocator are valid. Ineligible namespace/area/malformed
 * identities produce false. No existence claim or source loading is made.
 * On resource failure output remains unchanged; all local scratch is retired.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) unions all finite
 * matching decompositions rather than selecting a preferred version.
 */
DependencySelectorStatus dependency_selector_match(const DependencySelector *selector,
    const unsigned char *bytes, size_t length, SourceAllocator allocator, bool *output, SourceIssue *issue);
#endif
