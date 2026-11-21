/**
 * @file
 * @brief Share exact identity ordering, membership and source/test unit predicates.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) fixes identity sets;
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) fixes test association;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves sites.
 */
#ifndef AMA_SOURCE_CONFIGURATION_SELECTION_INTERNAL_H
#define AMA_SOURCE_CONFIGURATION_SELECTION_INTERNAL_H
#include "selection.h"
/** Exact bounded byte order under [PKG-05](../../../../spec/project/packages.md#pkg-05). */
int selection_compare(const char *a, size_t an, const char *b, size_t bn);
/** Binary membership under [PKG-05](../../../../spec/project/packages.md#pkg-05). */
const DependencyViewEntry *selection_find(const DependencyView *view, const char *bytes, size_t length);
/** Validate sorted uniqueness and version membership under [PKG-05](../../../../spec/project/packages.md#pkg-05). */
DependencySelectionStatus selection_validate(const DependencyView *view, SourceIssue *issue);
/** Preserve original issue sites under [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
DependencySelectionStatus selection_failure(SourceIssue *issue, SourceSpan span,
    DependencySelectionStatus status, const char *message, const char *expected);
/** Whole-component prefix relation under [PKG-05](../../../../spec/project/packages.md#pkg-05). */
bool selection_prefix(const char *prefix, size_t prefix_length, const char *value, size_t length, bool proper);
/** Exact version equality under [PKG-05](../../../../spec/project/packages.md#pkg-05). */
bool selection_same_version(DependencyViewEntry a, DependencyViewEntry b);
/** Proper mirrored test-directory descent under [PKG-08](../../../../spec/project/packages.md#pkg-08). */
bool selection_associated(DependencyViewEntry source, DependencyViewEntry test);
#endif
