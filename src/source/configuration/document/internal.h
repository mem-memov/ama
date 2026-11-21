/**
 * @file
 * @brief Separate INI document orchestration, sections, entries and cleanup.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) gives one schema;
 * [ARCH-01](../../../../spec/implementation/compiler.md#arch-01) keeps typed
 * state transitions separate from physical-line/record recognition.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) locates schema refusals;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial allocation cleanup.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04),
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) and
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) distinguish alias, selector and repository entry operations.
 */
#ifndef AMA_SOURCE_CONFIGURATION_DOCUMENT_INTERNAL_H
#define AMA_SOURCE_CONFIGURATION_DOCUMENT_INTERNAL_H
#include "document.h"
#include "source/configuration/records/internal.h"

/** Finite section state; module profile starts in its implicit alias table.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) selects one explicit finite section state for each entry.
 */
typedef struct {
    ConfigurationRecordKind kind;
    ConfigurationRepository *repository;
    bool seen_dependencies;
    bool seen_aliases;
} ConfigurationSection;

/** Record a schema refusal without changing any borrowed bytes.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains the original span and actionable explanation.
 */
bool configuration_document_failure(ConfigurationDocument *document, SourceSpan span,
    const char *message, const char *expected);
/** Allocate one fixed-sized arena record or retain an explicit allocation failure.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps successful partial allocations under the document destructor.
 */
void *configuration_document_allocate(ConfigurationDocument *document, size_t size, SourceSpan span);
/** Finish the preceding repository and admit a unique profile-appropriate header.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) forbids duplicate sections while preserving order independence.
 */
bool configuration_section_begin(ConfigurationDocument *document, ConfigurationSection *section,
    const ConfigurationRecord *record);
/** Require all mandatory fields of an existing repository; null means no pending repository.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) requires url and directory before repository completion.
 */
bool configuration_repository_complete(ConfigurationDocument *document, const ConfigurationRepository *repository);
/** Validate and append an exact unique alias in the document's profile.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) requires whole-key uniqueness and profile-appropriate targets.
 */
bool configuration_document_alias(ConfigurationDocument *document, const ConfigurationRecord *record);
/** Validate and append a unique dependency pattern without resolving it.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) keeps syntax collection separate from dependency acquisition.
 */
bool configuration_document_selector(ConfigurationDocument *document, const ConfigurationRecord *record);
/** Admit exactly one known repository field without unquoting its value.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) defines the fixed field inventory and preserves raw values.
 */
bool configuration_document_repository_field(ConfigurationDocument *document,
    ConfigurationRepository *repository, const ConfigurationRecord *record);
#endif
