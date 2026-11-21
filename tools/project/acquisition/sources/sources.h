/** @file
 * @brief Own coherent operation-local snapshot requests and selected version sources.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) resolves each exact
 * URL/revision request once and forbids mixed origins for one logical version.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) makes prepared versions
 * authoritative before remote routing. No transport runs in this component.
 */
#ifndef AMA_PROJECT_ACQUISITION_SOURCES_H
#define AMA_PROJECT_ACQUISITION_SOURCES_H
#include "../location/location.h"
#include "source/module/packages/packages.h"
/** Explicit pending is healthy; refusal never publishes a partial public record. */
typedef enum { ACQUISITION_SOURCES_OK, ACQUISITION_SOURCES_PENDING,
    ACQUISITION_SOURCES_INVALID, ACQUISITION_SOURCES_MISSING,
    ACQUISITION_SOURCES_CONFLICT, ACQUISITION_SOURCES_LIMIT,
    ACQUISITION_SOURCES_OUT_OF_MEMORY } AcquisitionSourcesStatus;
/** Owned exact transport key. revision NULL requests the remote default branch.
 * commit is initially NULL and becomes immutable after explicit publication.
 * Consumers borrow this record; only the owning operation mutates its commit. */
typedef struct {
    const char *url;
    const char *revision;
    const char *commit;
    SourceSpan origin;
} AcquisitionSnapshot;
/** One immutable per-version source contract. snapshot NULL denotes a selected
 * prepared copy; otherwise URL/commit/directory determine the remote contract. */
typedef struct {
    const char *version;
    const char *directory;
    const AcquisitionSnapshot *snapshot;
    const ModulePackage *local;
    SourceSpan origin;
} AcquisitionVersionSource;
/** Static primary explanation and optional prior coherent-version witness. */
typedef struct { SourceIssue issue; const AcquisitionVersionSource *previous; } AcquisitionSourcesIssue;
struct AcquisitionSnapshotRecord;
struct AcquisitionVersionRecord;
/** Single-executor operation owner. Prepared records/source owners are borrowed
 * through consumption. Snapshot/version records and failed scratch are owned
 * until destroy; transport results must be serialized before publication. */
typedef struct {
    SourceArena arena;
    const ModulePackage *prepared;
    struct AcquisitionSnapshotRecord *snapshots;
    struct AcquisitionVersionRecord *versions;
    size_t snapshot_count, version_count;
} AcquisitionSources;
/** Prepared on-disk copy from either storage root, or a pending/known remote
 * source, borrowing the operation.
 * resolve_snapshot is true only for first insertion of an exact transport key;
 * the caller uses it to schedule one explicit Git-resolution job. */
typedef struct {
    const ModulePackage *local;
    const AcquisitionLocation *location;
    const AcquisitionSnapshot *snapshot;
    bool resolve_snapshot;
    SourceSpan origin;
} AcquisitionSourceRequest;
/** Begin one operation with the successful common PKG-02 selected-version list.
 * A NULL list represents no prepared versions. No source indexing is required. */
void acquisition_sources_init(AcquisitionSources *, SourceAllocator, const ModulePackage *);
/** Release complete/failed storage; expire all operation-owned returned handles. */
void acquisition_sources_destroy(AcquisitionSources *);
/** Choose a prepared version before consulting scope. Otherwise select/map only
 * this request's scope and intern the exact URL/optional-revision snapshot key.
 * Original path/configuration owners outlive consumption. No I/O or Git occurs. */
AcquisitionSourcesStatus acquisition_sources_request(AcquisitionSources *, const ConfigurationDocument *,
    const ModulePathValue *, AcquisitionSourceRequest *output, AcquisitionSourcesIssue *);
/** Publish an externally validated resolved commit identifier once. Equal repeat
 * publication is idempotent; unequal publication refuses and preserves the first
 * commit. Unknown/foreign handles refuse before dereference. No Git is invoked. */
AcquisitionSourcesStatus acquisition_snapshot_resolve(AcquisitionSources *, const AcquisitionSnapshot *,
    const char *commit, AcquisitionSourcesIssue *);
/** Bind a successful request from this operation to its coherent version. Unresolved snapshot returns PENDING and
 * leaves output/registry unchanged. Equal URL/commit/physical-directory contracts
 * share one record, including distinct revision names for the same commit.
 * Conflict preserves all existing contracts and reports the prior witness. */
AcquisitionSourcesStatus acquisition_sources_bind(AcquisitionSources *, const AcquisitionSourceRequest *,
    const AcquisitionVersionSource **output, AcquisitionSourcesIssue *);
#endif
