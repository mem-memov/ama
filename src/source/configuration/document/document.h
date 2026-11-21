/**
 * @file
 * @brief Own one strict package/module INI document and original record views.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) defines the two profiles;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps failed
 * preparation under the same explicit destructor as complete documents.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) supplies alias records,
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) supplies selector patterns,
 * and [PKG-06](../../../../spec/project/packages.md#pkg-06) supplies repository fields.
 */
#ifndef AMA_SOURCE_CONFIGURATION_DOCUMENT_H
#define AMA_SOURCE_CONFIGURATION_DOCUMENT_H
#include "source/configuration/records/records.h"
#include "source/memory/arena.h"

/** Package documents have explicit sections; module documents have only aliases.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) separates explicit package sections from sectionless module aliases.
 */
typedef enum { CONFIGURATION_PACKAGE, CONFIGURATION_MODULE } ConfigurationProfile;
/** Invalid syntax/schema is distinct from allocator refusal; only OK publishes valid.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed ownership without publishing partial validity.
 */
typedef enum { CONFIGURATION_OK, CONFIGURATION_INVALID, CONFIGURATION_OUT_OF_MEMORY } ConfigurationStatus;

/** Original exact alias key and raw target; scope resolution is not a parsing effect.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids package targets
 * from containing aliases, while local targets may name a package alias.
 */
typedef struct ConfigurationAlias {
    struct ConfigurationAlias *next;
    SourceSpan name;
    SourceSpan target;
} ConfigurationAlias;

/** Original canonical selector pattern, without expansion or downloaded input.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) leaves set matching to
 * the shared resolver rather than this source-document parser.
 */
typedef struct ConfigurationSelector {
    struct ConfigurationSelector *next;
    SourceSpan pattern;
} ConfigurationSelector;

/** One complete repository record, retaining unquoted raw value bytes.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) requires url/directory;
 * an absent revision has a null source, not an invented default branch name.
 */
typedef struct ConfigurationRepository {
    struct ConfigurationRepository *next;
    SourceSpan header;
    SourceSpan prefix;
    SourceSpan url;
    SourceSpan directory;
    SourceSpan revision;
} ConfigurationRepository;

/** Parse-once owner; records and issue views borrow source until destruction.
 * Do not copy a live owner; initialize a separate owner for each parse.
 * Partial lists are not consumable on refusal; their storage is nevertheless
 * always arena-owned. No filesystem, network, environment or instance effect.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) preserves original text.
 */
typedef struct {
    SourceArena arena;
    const SourceText *source;
    ConfigurationAlias *aliases;
    ConfigurationAlias *last_alias;
    ConfigurationSelector *selectors;
    ConfigurationSelector *last_selector;
    ConfigurationRepository *repositories;
    ConfigurationRepository *last_repository;
    ConfigurationIssue issue;
    ConfigurationStatus status;
    ConfigurationProfile profile;
    bool attempted;
    bool valid;
} ConfigurationDocument;

/** Initialize an empty document borrowing allocator authority through destruction.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) gives each document one explicit allocator/destructor authority.
 */
void configuration_document_init(ConfigurationDocument *document, SourceAllocator allocator);
/** Retire complete or failed arena storage; source bytes are borrowed and never freed.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates owned records from borrowed source storage.
 */
void configuration_document_destroy(ConfigurationDocument *document);
/** Parse once from a fresh initialized owner and immutable source.
 * Source outlives every record/issue view. On failure valid stays false, no input
 * changes, and the same destructor releases every successful partial allocation.
 * Empty documents/optional sections are valid; required repository fields and
 * complete exact duplicate keys/sections are checked before success.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) forbids guessing headers,
 * interpolation, unknown records and duplicate configuration entries.
 */
ConfigurationStatus configuration_document_parse(ConfigurationDocument *document,
    const SourceText *source, ConfigurationProfile profile);
#endif
