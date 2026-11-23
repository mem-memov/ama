/** @file
 * @brief Join actual command placement, original copy closure and designated version proposals.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * fixes explicit physical seeds and propagates their designated versions; final
 * discovery and reference preservation remain independent acceptance gates.
 */
#ifndef AMA_PROJECT_SOURCE_SEEDS_H
#define AMA_PROJECT_SOURCE_SEEDS_H
#include "../../../source/copy/destinations/destinations.h"
#include "../../../source/mapping/propose.h"
#include "../../../source/mapping/physical/version.h"
/** Keep original closure, explicit placement, logical and storage failures distinct. */
typedef enum { PROJECT_SOURCE_SEEDS_OK, PROJECT_SOURCE_SEEDS_INVALID,
    PROJECT_SOURCE_SEEDS_CLOSURE, PROJECT_SOURCE_SEEDS_EXPLICIT,
    PROJECT_SOURCE_SEEDS_DESTINATIONS, PROJECT_SOURCE_SEEDS_VERSION,
    PROJECT_SOURCE_SEEDS_POLICY, PROJECT_SOURCE_SEEDS_MEMORY } ProjectSourceSeedsStatus;
/** Own closure and seed metadata; borrow original analysis/capture, placement,
 * declarations and final roots through destruction. valid certifies candidates
 * only, never complete C/P, final selection, replacement or publication. */
typedef struct {
    SourceArena arena;
    SourceCopyClosure copy;
    SourceCloneMap explicit_mains;
    SourceCopyDestinations destinations;
    SourceVersionPlacement *versions;
    size_t version_count;
    const ModuleRoot *roots;
    size_t root_count;
    const SourceTreePlacement *placement;
    SourceCloneTreeIssue explicit_issue;
    const SourceCloneMapping *failed_row;
    const char *failed_path;
    ProjectSourceSeedsStatus status;
    bool attempted, valid;
} ProjectSourceSeeds;
/** Begin empty owners without filesystem or compiler work. */
void project_source_seeds_init(ProjectSourceSeeds *, SourceAllocator);
/** Retire destination/closure borrowers before original inputs and seed strings. */
void project_source_seeds_destroy(ProjectSourceSeeds *);
/** Build once from actual prepared placement over the same admitted original
 * analysis. Reuse exact raw-to-main proposals and the shared finite closure and
 * logical solver. Ordinary versions derive physical authority solely from actual
 * explicit placements, including drafts; duplicate equal authorities coalesce,
 * different authorities refuse. Cross-version identity changes require an
 * explicit dependency policy. No source writes, network or code execution; shared
 * closure may read/check required original unit/test inputs. No recapture fallback.
 * Failure retains owned invalid scratch; repeated calls preserve prior evidence.
 * Finite shared algorithms plus quadratic seed/version comparisons and exact
 * path-component scans bound time; storage is child-owned scratch and O(seed
 * path bytes). Final roots/whole-version selection, topology/context, complete P,
 * cuts, edits and the complete final certificate remain mandatory outer gates.
 */
ProjectSourceSeedsStatus project_source_seeds_build(ProjectSourceSeeds *, const SourceTreePlacement *,
    SourceAnalysis *, const SourceDeclaredGroup *, SourceCopyDependencyMode, const ModuleRoot *, size_t);
#endif
