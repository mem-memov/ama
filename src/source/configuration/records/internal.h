/**
 * @file
 * @brief Share exact record recognition and typed INI spelling predicates.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) validates syntax before
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) resolves alias scope.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) attributes malformed entries to original spans.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) supplies selector spelling;
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) supplies repository prefix/directory constraints.
 */
#ifndef AMA_SOURCE_CONFIGURATION_RECORDS_INTERNAL_H
#define AMA_SOURCE_CONFIGURATION_RECORDS_INTERNAL_H
#include "records.h"

/** Compare a bounded original view with one complete static ASCII spelling.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) fixes complete header/key spellings, never partial matches.
 */
bool configuration_text_is(SourceSpan text, const char *literal);
/** Compare complete original keys byte-for-byte, without locale or prefix matching.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) preserves complete alias names without normalization.
 */
bool configuration_text_equal(SourceSpan left, SourceSpan right);
/** Fill the first caller-owned issue with this exact span and static explanations.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) attributes refusal to original configuration text.
 */
bool configuration_record_failure(SourceIssue *issue, SourceSpan span,
    const char *message, const char *expected);
/** Recognize an exact known header and validate repository prefix spelling.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) constrains the repository prefix before any remote lookup.
 */
bool configuration_header(SourceSpan line, ConfigurationRecord *record, SourceIssue *issue);
/** Split the first equals sign at its exact one-space joins; later equals signs are data.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) preserves subsequent equals signs as value data.
 */
bool configuration_assignment(SourceSpan line, size_t equal, ConfigurationRecord *record,
    SourceIssue *issue);
/** Validate a complete tilde-prefixed ordinary alias key without resolving it.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) requires tilde plus one complete ordinary name.
 */
bool configuration_alias_key(SourceSpan text);
/** Validate a raw direct/module alias target; package targets cannot start with tilde.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) permits package aliases only in a module target.
 */
bool configuration_alias_target(SourceSpan text, bool allow_alias);
/** Validate only selector syntax; matching/alias expansion remains a separate operation.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) separates pattern syntax from selection effects.
 */
bool configuration_selector(SourceSpan text);
/** Validate a slash-rooted vendor/package prefix containing no version component.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) ends repository namespace prefixes before the version.
 */
bool configuration_repository_prefix(SourceSpan text);
/** Check slash-relative physical directory containment lexically, without filesystem access.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) forbids checkout escape; filesystem evidence remains a later acquisition check.
 */
bool configuration_repository_directory(SourceSpan text);
#endif
