/** @file
 * @brief Own byte-exact prepared relocation sources and their independently resolved final graph.
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * preserves non-path bytes; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires final validation before filesystem publication.
 */
#ifndef AMA_RELOCATION_STAGING_H
#define AMA_RELOCATION_STAGING_H
#include "../references/references.h"
#include "library/model.h"
/** One captured source's prepared text; inactive drafts have identical bytes under their final physical path. Component contract: [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RelocationStagedSource {
    struct RelocationStagedSource *next;
    const RelocationModule *module;
    const SourceText *text;
    const RelocationPathEdit **edits;
    size_t edit_count;
} RelocationStagedSource;
/** One inventoried file/directory/link or external source dependency.
 * Regular bytes and literal link-target bytes are copied; directory byte views are null.
 * replacement retains original bytes except for prepared Ama sources. Paths borrow
 * preview/reference/source owners, all of which outlive this staging owner.
 * Component contract: [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RelocationStagedFile {
    struct RelocationStagedFile *next;
    const char *before;
    const char *after;
    struct stat metadata;
    const SourceText *original;
    const SourceText *replacement;
    bool protected_input;
} RelocationStagedFile;
/** Explicit documented settings; no configuration format is inferred from filenames.
 * Optional entry is an original analysis identity. Library files use the existing
 * ama-library parser. Generated outputs are explicit physical paths with existing
 * parents; absence of their final component is a valid no-op invalidation.
 * Component contract: [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct {
    const RelocationReferences *references;
    const char *entry;
    const char *const *library_configurations;
    size_t library_configuration_count;
    const char *const *generated_outputs;
    size_t generated_output_count;
} RelocationStagingRequest;
/** Both parsed library models own their allocations and borrow the corresponding semantic snapshot. Component contract: [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RelocationStagedConfiguration {
    struct RelocationStagedConfiguration *next;
    RelocationStagedFile *file;
    LibraryBuild before;
    LibraryBuild after;
} RelocationStagedConfiguration;
/** Explicit generated output invalidation queued for the later recoverable transaction. Component contract: [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RelocationInvalidation {
    struct RelocationInvalidation *next;
    const char *before;
    const char *after;
    struct stat metadata;
    bool existed;
} RelocationInvalidation;
/** Single-attempt read-only owner; references and its preview outlive this owner.
 * Text, catalog, AST and semantic views survive failure until destruction. Only
 * ready permits consuming the complete final graph; publication is a separate operation.
 * Component contract: [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct {
    const RelocationReferences *references;
    SourceArena arena;
    SourceManager sources;
    Project project;
    SemanticProgram semantic;
    RelocationStagedSource *first;
    RelocationStagedSource *last;
    RelocationStagedFile *files;
    RelocationStagedFile *last_file;
    RelocationStagedConfiguration *configurations;
    RelocationInvalidation *invalidations;
    const char *entry;
    RelocationStatus status;
    Diagnostic error;
    DiagnosticRelated related;
    bool semantic_initialized;
    bool attempted;
    bool ready;
} RelocationStaging;
/** Initialize independent storage, borrowing allocator authority through destruction. Component contract: [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_staging_init(RelocationStaging *, SourceAllocator);
/** Release semantic views before final source/catalog storage, including failed prefixes. Component contract: [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_staging_destroy(RelocationStaging *);
/** Prepare source bytes and independently prove the final graph once, without changing files. Component contract: [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_build(RelocationStaging *, const RelocationStagingRequest *);
#endif
