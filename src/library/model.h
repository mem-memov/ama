/** @file
 * @brief Own explicit component exports, transport declarations and their checked project.
 * [EMBED-01](../../spec/implementation/c-library.md#embed-01) keeps library
 * selection separate from executable launchers and implicit filename exports.
 */
#ifndef AMA_LIBRARY_MODEL_H
#define AMA_LIBRARY_MODEL_H
#include <stdio.h>
#include <stdint.h>
#include "compiler/frontend.h"
#include "source/identity/identity.h"
/** Supported independently owned wire values; references require another explicit policy. */
typedef enum { LIBRARY_INTEGER, LIBRARY_TEXT, LIBRARY_CONDITION, LIBRARY_FAILURE,
    LIBRARY_LIST } LibrarySchemaKind;
/** Explicit public operation set; an advertised resume still requires saved state. */
typedef enum { LIBRARY_ENTRY = 1, LIBRARY_RESUME = 2, LIBRARY_READ = 4 } LibraryOperation;
/** Named schema declaration; list elements refer to another non-list scalar schema. */
typedef struct LibrarySchema {
    struct LibrarySchema *next;
    const char *name;
    const char *argument;
    LibrarySchemaKind kind;
    SourceSpan site;
    struct LibrarySchema *element;
    const SemanticType *type;
    const IdentityName *identity;
} LibrarySchema;
/** One explicit exported type; operations and transport mappings never come from its basename. */
typedef struct LibraryExport {
    struct LibraryExport *next;
    const char *name;
    const char *module;
    unsigned operations;
    SourceSpan site;
    const SemanticType *type;
    const IdentityName *identity;
} LibraryExport;
/** One exact interface-name/schema binding; declaration order is retained as role_ordinal. */
typedef struct LibraryBinding {
    struct LibraryBinding *next;
    const char *export_name;
    const char *field_name;
    const char *schema_name;
    bool input;
    SourceSpan site;
    const LibraryExport *exported;
    const LibrarySchema *schema;
    const SemanticSymbol *field;
} LibraryBinding;
/** Ordered explicit compiler/link input, preserving every byte after its record separator. */
typedef struct LibrarySetting {
    struct LibrarySetting *next;
    const char *kind;
    const char *value;
    SourceSpan site;
} LibrarySetting;
/** Single-attempt owner. Views borrow its arena/source/frontend through destruction.
 * Invalid and partially loaded models remain destructible and cannot be consumed.
 */
typedef struct {
    SourceArena arena;
    SourceManager sources;
    IdentityRegistry identities;
    CompilerFrontend frontend;
    const SourceText *configuration;
    const char *component;
    const char *abi;
    const char *target;
    SourceSpan target_site;
    const char *calling_convention;
    const IdentityName *identity;
    const char **identity_parts;
    size_t identity_part_count;
    LibrarySchema *schemas;
    LibraryExport *exports;
    LibraryBinding *bindings;
    LibrarySetting *settings;
    size_t export_count;
    Diagnostic error;
    DiagnosticRelated related;
    bool format_seen;
    bool parsed;
    bool attempted;
    bool valid;
} LibraryBuild;
/** Initialize all independent owners; allocator authority survives destruction. */
void library_build_init(LibraryBuild *, SourceAllocator);
/** Release the typed project before configuration and identity storage; invalidate views. */
void library_build_destroy(LibraryBuild *);
/** Parse one borrowed UTF-8 configuration without source discovery or output writes.
 * Failed partial records remain owned; the source must outlive destruction.
 */
bool library_configuration_parse(LibraryBuild *, const SourceText *);
/** Resolve an already parsed explicit export configuration against exact configured roots.
 * Capture configuration/link dependencies before sealing the frontend input graph.
 * No native compilation, source overwrite, launcher generation or execution occurs.
 */
bool library_build_resolve(LibraryBuild *, const ModuleRoot *, size_t);
/** Load the configuration into this owner's source manager, then parse and resolve once. */
bool library_build_check(LibraryBuild *, const ModuleRoot *, size_t, const char *);
/** Write deterministic resolved metadata to a borrowed stream; require a valid build.
 * Propagate partial-write/flush failure. The caller owns publication policy.
 */
bool library_manifest_write(const LibraryBuild *, FILE *);
/** Require a valid resolved model and validate its explicit consumer C-ABI profile; failure optionally supplies a source-backed diagnostic. */
bool library_header_target(const LibraryBuild *, Diagnostic *);

/** Write a self-contained C11 consumer header from one checked export model; no private headers or runtime addresses escape. */
bool library_header_write(const LibraryBuild *, FILE *);

/** Write the allocation-free ABI bootstrap definition after its public header has been included; no Ama context is accessed. */
bool library_abi_write(const LibraryBuild *, FILE *);

/** Emit private lifecycle definitions after the matching public header and generated project assembly.
 * types supplies the exact nonzero project descriptor ordinal for each export in declaration order.
 * Count/profile validation precedes output; no body, context or instance is executed by generation.
 */
bool library_lifecycle_write(const LibraryBuild *, const uint64_t *types, size_t count, FILE *);

/** Emit immutable wire-owner definitions after the matching public header and lifecycle result helpers.
 * No context/native payload is retained; output failure propagates without publication policy.
 */
bool library_snapshots_write(const LibraryBuild *, FILE *);

/** Write the exact externally visible definition inventory for ELF object isolation.
 * One symbol per line, in declaration order. Undefined C dependencies are not
 * definitions and must remain unresolved for the final host linker.
 */
bool library_symbols_write(const LibraryBuild *, FILE *);

#endif
