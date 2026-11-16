/** @file
 * @brief Own a read-only directory relocation preview over explicit source roots.
 * [TOOL-01](../../../spec/implementation/tools-and-conformance.md#tool-01)
 * gives D the meaning of S's exact final location, including carried roots.
 * [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates preparation from later recoverable publication.
 */
#ifndef AMA_TOOLS_RELOCATION_H
#define AMA_TOOLS_RELOCATION_H
#include <sys/stat.h>
#include "source/module/catalog.h"
#include "ast/ast.h"

/** Distinct preflight failures never authorize a partially prepared plan. */
typedef enum {
    RELOCATION_OK, RELOCATION_OUT_OF_MEMORY, RELOCATION_INVALID_PATH,
    RELOCATION_IO_ERROR, RELOCATION_UNSUPPORTED_PATH, RELOCATION_DESTINATION_EXISTS,
    RELOCATION_DESCENDANT, RELOCATION_UNCOVERED, RELOCATION_COLLISION,
    RELOCATION_ALIAS, RELOCATION_ORPHAN, RELOCATION_READ_ONLY, RELOCATION_INVALID_SOURCE,
    RELOCATION_INVALID_REFERENCE, RELOCATION_REFERENCE_CONFLICT, RELOCATION_CHANGED_INPUT,
    RELOCATION_BUSY, RELOCATION_INCOMPLETE_TRANSACTION
} RelocationStatus;
/** Original and projected physical identities of an accompanying tree entry; no link is followed. */
typedef struct RelocationEntry {
    struct RelocationEntry *next;
    const char *before;
    const char *after;
    struct stat metadata;
} RelocationEntry;
/** One source candidate and its exact projected identity, retaining original bytes for later edits.
 * A null before_identity denotes a newly introduced source, not a guessed original namespace.
 */
typedef struct RelocationModule {
    struct RelocationModule *next;
    const char *before_path;
    const char *after_path;
    const char *before_identity;
    const char *after_identity;
    const SourceText *text;
    dev_t device;
    ino_t inode;
    bool active;
} RelocationModule;
/** Borrowed inputs are copied during build. destination_root, when supplied, describes its final location. */
typedef struct {
    const ModuleRoot *roots;
    size_t root_count;
    const char *source;
    const char *destination;
    const ModuleRoot *destination_root;
} RelocationRequest;
/** Build-once preview owner. Views survive a failed build until destruction; ready alone
 * permits consuming complete results. This core performs no filesystem mutation.
 */
typedef struct {
    SourceArena arena;
    ModuleCatalog catalog;
    SourceManager additional_sources;
    const char *source;
    const char *destination;
    const ModuleRoot *before_roots;
    const ModuleRoot *after_roots;
    size_t before_root_count;
    size_t after_root_count;
    RelocationEntry *entries;
    RelocationEntry *last_entry;
    RelocationModule *modules;
    RelocationModule *last_module;
    RelocationStatus status;
    const char *path;
    const char *related_path;
    Diagnostic diagnostic;
    DiagnosticRelated catalog_related;
    bool attempted;
    bool ready;
} RelocationPreview;
/** Initialize independent retained allocation/source owners; allocator authority outlives the preview. */
void relocation_preview_init(RelocationPreview *, SourceAllocator);
/** Release complete or failed preparation, invalidating every borrowed result. */
void relocation_preview_destroy(RelocationPreview *);
/** Prepare exact paths, roots, physical entries and all configured source candidates without mutation.
 * Failure retains its first diagnostic and owned prefix, never a consumable complete plan.
 */
bool relocation_preview_build(RelocationPreview *, const RelocationRequest *);
#endif
