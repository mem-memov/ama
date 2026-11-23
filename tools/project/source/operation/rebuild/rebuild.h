/** @file
 * @brief Own post-commit rebuild intentions without editing generated or unknown external files.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * invalidates generated C, source maps, native prefixes and caches instead of
 * patching embedded paths; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * records this intention in the same finite operation before publication.
 */
#ifndef AMA_SOURCE_OPERATION_REBUILD_H
#define AMA_SOURCE_OPERATION_REBUILD_H
#include "../roots/roots.h"
#include "../clients/clients.h"
/** Preserve all three semantic boundaries for consumers and completion reports. */
typedef enum { SOURCE_OPERATION_DOMAIN_CONFIGURED, SOURCE_OPERATION_DOMAIN_ORIGINAL,
    SOURCE_OPERATION_DOMAIN_FINAL } SourceOperationRebuildDomain;
/** One complete build-input domain to invalidate on successful publication.
 * Repeated/equivalent descriptors remain separate provenance, not duplicate
 * physical deletion requests. This component never invents artifact locations. */
typedef struct SourceOperationRebuildScope {
    struct SourceOperationRebuildScope *next;
    const ModuleRoot *root;
    SourceOperationRebuildDomain domain;
} SourceOperationRebuildScope;
/** Rebuild through normal producers; never rewrite an artifact in place. */
typedef enum { SOURCE_OPERATION_REBUILD_C = 1, SOURCE_OPERATION_REBUILD_SOURCE_MAPS = 2,
    SOURCE_OPERATION_REBUILD_NATIVE_PREFIXES = 4, SOURCE_OPERATION_REBUILD_CACHES = 8,
    SOURCE_OPERATION_REBUILD_TEST_OUTPUTS = 16 } SourceOperationRebuildClass;
/** Known semantic boundaries, retained for preview/completion rather than guessed repairs. */
typedef enum { SOURCE_OPERATION_OUTSIDE_IMPORTERS_UNCHECKED = 1,
    SOURCE_OPERATION_OPAQUE_EMBEDDED_PATHS_UNREWRITTEN = 2,
    SOURCE_OPERATION_EXTERNAL_CLIENT_FORMATS_UNWRITTEN = 4 } SourceOperationReportBoundary;
/** Distinguish malformed owners, representation exhaustion and allocation failure. */
typedef enum { SOURCE_OPERATION_REBUILD_OK, SOURCE_OPERATION_REBUILD_INVALID,
    SOURCE_OPERATION_REBUILD_LIMIT, SOURCE_OPERATION_REBUILD_MEMORY } SourceOperationRebuildStatus;
/** Own report rows and borrow the same exact settings/client plan through
 * destruction. Domain-wide invalidation is deliberately conservative: every
 * reusable build whose inputs use O, U or F must be revalidated/regenerated.
 * It avoids a second, incomplete reachability model for unused/static/native/test
 * consumers. No runtime execution, invalidation effect or deletion occurs now. */
typedef struct {
    SourceArena arena;
    const SourceOperationRoots *roots;
    const SourceOperationClients *clients;
    SourceOperationRebuildScope *first, *last;
    size_t count;
    unsigned classes, boundaries;
    SourceOperationRebuildStatus status;
    bool attempted, valid;
} SourceOperationRebuild;
/** Begin empty report ownership without effects. */
void source_operation_rebuild_init(SourceOperationRebuild *, SourceAllocator);
/** Retire all success/failure row prefixes without touching generated outputs. */
void source_operation_rebuild_destroy(SourceOperationRebuild *);
/** Build once from accepted root/client reports of the identical prepared plan.
 * Include all O/U/F descriptors, including explicit dropped/added roots and
 * roots with only ignored or unused inputs. Emit every rebuild class and the
 * stated source-rewrite boundaries. No I/O, path guessing, hidden output discovery
 * or writable external configuration assumption. For N=|O|+|U|+|F|, checked
 * representation, time and owned row storage are O(N). Failure remains invalid
 * with owned scratch; repeat refuses without erasing successful output.
 * Publication/recovery must consume this intention; this stage is not evidence
 * that any build cache or artifact has already been invalidated.
 */
SourceOperationRebuildStatus source_operation_rebuild_build(SourceOperationRebuild *,
    const SourceOperationRoots *, const SourceOperationClients *);
#endif
