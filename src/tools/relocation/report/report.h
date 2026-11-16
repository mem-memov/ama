/** @file
 * @brief Own byte-exact relocation scope and outcome records independently of compiler snapshots.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * reports identities, rewritten files, preserved drafts and rebuild requirements after application or recovery.
 */
#ifndef AMA_RELOCATION_REPORT_H
#define AMA_RELOCATION_REPORT_H
#include "../staging/staging.h"

/** One source's exact simultaneous identity/placement mapping; null before_identity denotes explicitly introduced scope. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RelocationReportedModule {
    struct RelocationReportedModule *next;
    const char *before;
    const char *after;
    const char *before_identity;
    const char *after_identity;
    bool active;
} RelocationReportedModule;
/** One contributing physical entry, with explicit semantic rewrite/draft/output roles rather than filename guesses. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RelocationReportedFile {
    struct RelocationReportedFile *next;
    const char *before;
    const char *after;
    mode_t mode;
    bool existed;
    bool rewritten;
    bool draft;
    bool opaque;
    bool invalidated;
} RelocationReportedFile;
/** Independently owned report; physical path strings retain raw filesystem bytes and logical identities retain exact Unicode bytes.
 * Only ready permits consumption. Outcome is supplied by the application/recovery state,
 * not inferred from a later workspace scan. External flags/C paths remain outside rewrites.
 * Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct {
    SourceArena arena;
    const char *source;
    const char *destination;
    const char *entry;
    const ModuleRoot *before_roots;
    const ModuleRoot *after_roots;
    size_t before_root_count;
    size_t after_root_count;
    RelocationReportedModule *modules;
    RelocationReportedFile *files;
    RelocationStatus status;
    bool attempted;
    bool ready;
    bool rebuild_required;
} RelocationReport;
/** Initialize independent ownership without reading source or creating output. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_report_init(RelocationReport *, SourceAllocator);
/** Invalidate all report views and release every successful or failed allocation prefix. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_report_destroy(RelocationReport *);
/** Copy complete plan scope once, preserving bytes and explicit roles without executing native/Ama code. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_report_build(RelocationReport *, const RelocationStaging *);
#endif
