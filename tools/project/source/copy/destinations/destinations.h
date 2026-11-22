/** @file
 * @brief Join original copy-closure causes with explicit and associated logical destinations.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves explicit placements, propagates designated versions, and makes tests
 * follow their deepest mapped source association. Final physical admission is separate.
 */
#ifndef AMA_PROJECT_SOURCE_COPY_DESTINATIONS_H
#define AMA_PROJECT_SOURCE_COPY_DESTINATIONS_H
#include "../closure.h"
#include "../../mapping/map.h"
#include "../../mapping/versions/plan.h"
/** Active ordinary logical candidate; source/role borrow the admitted original
 * closure, while version/identity bytes belong to this result's arena.
 */
typedef struct {
    const ModuleSource *source;
    const AcquisitionRoleRecord *role;
    const char *version, *identity;
} SourceCopyDestination;
/** Namespace incompatibility is distinct from conflicting/unknown version demands.
 * This ordinary-version phase does not define standard-root physical placement.
 */
typedef enum { SOURCE_COPY_DESTINATIONS_OK, SOURCE_COPY_DESTINATIONS_INVALID,
    SOURCE_COPY_DESTINATIONS_NAMESPACE, SOURCE_COPY_DESTINATIONS_MEMORY,
    SOURCE_COPY_DESTINATIONS_VERSIONS, SOURCE_COPY_DESTINATIONS_NAMES } SourceCopyDestinationsStatus;
/** Own candidate metadata over immutable copy/explicit-tree inputs. valid proves
 * active ordinary logical destinations only. Context-only anchors, draft mains
 * retained solely as raw test/data material, opaque files, physical placement,
 * final whole-version selection, edits and transactions remain outer obligations.
 */
typedef struct {
    SourceArena arena;
    const SourceCopyClosure *copy;
    const SourceCloneMap *explicit_map;
    const SourceCopyDestination *entries;
    size_t count;
    SourceVersionStatus version_status;
    SourceVersionIssue version_issue;
    SourceCopyDestinationsStatus status;
    const AcquisitionRoleRecord *failed_role;
    bool attempted, valid;
} SourceCopyDestinations;
/** Begin result ownership; input owners must outlive inspection and teardown. */
void source_copy_destinations_init(SourceCopyDestinations *, SourceAllocator);
/** Retire scratch/candidates before borrowed closure and explicit-map owners. */
void source_copy_destinations_destroy(SourceCopyDestinations *);
/** Build once from a valid copy closure and exactly its explicit selected-tree
 * clone rows (one per raw selected indexed main, no extra rows). Both must refer
 * to the same original capture. Ordinary explicit units require ordinary final
 * module names in this phase; a standard/ordinary namespace crossing reports its
 * own stage for the outer root-kind planner. Standard originals have no ordinary
 * version candidate. No physical destination, package permission or operation
 * support is inferred from this phase alone.
 * Use active ordinary roles only. Topology-only reasons do not place copies.
 * Tests within an active checked source association follow its deepest original
 * associated directory; other added units follow their actual requiring roles.
 * Explicit target placement overrides incoming version demands. Derive source
 * names before associated test names, preserve outside helper suffixes, and refuse
 * conflicting/unresolved demands. Output is published only after all stages pass.
 * Failed prefixes stay owned; no I/O, compiler/test execution or source mutation.
 */
SourceCopyDestinationsStatus source_copy_destinations_build(SourceCopyDestinations *,
    const SourceCopyClosure *, const SourceCloneMap *explicit_map);
#endif
