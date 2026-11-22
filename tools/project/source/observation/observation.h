/** @file
 * @brief Capture original directory/name/metadata observations through the common catalog pipeline.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires an exact original scope; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses contradictory captures and keeps final transaction freshness separate.
 */
#ifndef AMA_PROJECT_SOURCE_OBSERVATION_H
#define AMA_PROJECT_SOURCE_OBSERVATION_H
#include "../capture/capture.h"
#include "../image/image.h"
struct SourceObservedEntry;
/** Preserve physical/read/catalog/consistency failure stages without recapture. */
typedef enum { SOURCE_OBSERVATION_OK, SOURCE_OBSERVATION_INVALID,
    SOURCE_OBSERVATION_MEMORY, SOURCE_OBSERVATION_PHYSICAL, SOURCE_OBSERVATION_CATALOG,
    SOURCE_OBSERVATION_CHANGED } SourceObservationStatus;
/** Explicit demands keep opaque/deep inputs separate from common selected discovery. */
typedef enum { SOURCE_OBSERVE_METADATA, SOURCE_OBSERVE_CONTENT, SOURCE_OBSERVE_CHILDREN } SourceObservationKind;
/** Borrow an exact catalog physical key through build; optional missing ancestors
 * prove absence without inventing unavailable descendant metadata. */
typedef struct { const char *path; SourceObservationKind kind; bool optional; } SourceObservationRequest;
/** Own read-only preparation scratch and immutable output bytes/rows. Borrow the
 * complete original capture until destruction. catalog is the common observed
 * selection used only as evidence, not a new mutable source-I/O entry point after
 * sealing. Image consumers expire before this owner. Directory observations are
 * complete where traversed; ignored/excluded names may have unavailable metadata
 * and bytes. The image never fills such holes from host storage after sealing.
 */
typedef struct {
    SourceArena arena;
    SourceManager files;
    ModuleCatalog physical, catalog;
    ModuleCatalogReader reader;
    SourceCatalogImage image;
    struct SourceObservedEntry *first, *last;
    size_t count;
    const SourceRootCapture *original;
    const char *failed_path;
    SourcePhysicalIssue physical_issue;
    SourceStatus file_status;
    SourceObservationStatus status;
    bool attempted, valid, sealed;
} SourceObservedImage;
/** Begin independent scratch/catalog/byte owners with borrowed allocator authority. */
void source_observed_image_init(SourceObservedImage *, SourceAllocator);
/** Retire catalog borrowers before captured files and record metadata. */
void source_observed_image_destroy(SourceObservedImage *);
/** Build once over the same complete already prepared original domain. Read local
 * filesystem inputs only; never execute code, fetch modules or mutate sources.
 * Record complete directory names through the common catalog traversal, preserving
 * unknown ignored entries. Independently compare selected main identities/bytes,
 * package configurations, optional aliases and every captured original companion;
 * reject new companions, missing originals or any different observed main set.
 * Do not replace the original capture on mismatch. Stable original paths/directories
 * are required throughout this synchronous observation, as for the physical catalog.
 * Explicit requests observe only their exact ancestor chains and demanded bytes
 * or immediate child lists, before sealing. Keys use the catalog root-plus-slash
 * spelling and forbid empty, dot and parent components below the root. Repeated
 * requests reuse observations; optional absence never hides I/O/type failures.
 * On success seal the observation callbacks and publish one immutable image.
 * Final whole-domain freshness before application is still mandatory. Failed
 * prefixes remain owned and never valid; repeated calls preserve previous results.
 */
SourceObservationStatus source_observed_image_build(SourceObservedImage *, const SourceRootCapture *,
    const SourceObservationRequest *, size_t);
#endif
