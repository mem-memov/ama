/** @file
 * @brief Prepare one common original catalog when final roots expose previously uncovered existing inputs.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires augmentation before edits, with original identities preserved and no
 * merger of independently analyzed catalogs. [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * keeps every failed capture and raw comparison alive for diagnostics.
 */
#ifndef AMA_PROJECT_SOURCE_AUGMENTATION_H
#define AMA_PROJECT_SOURCE_AUGMENTATION_H
#include "../roots/roots.h"
#include "../../../source/operation/roots/region.h"
/** Retain exact physical/coverage/capture/raw consistency failure boundaries. */
typedef enum { PROJECT_SOURCE_AUGMENTATION_OK, PROJECT_SOURCE_AUGMENTATION_INVALID,
    PROJECT_SOURCE_AUGMENTATION_MEMORY, PROJECT_SOURCE_AUGMENTATION_LIMIT,
    PROJECT_SOURCE_AUGMENTATION_PHYSICAL, PROJECT_SOURCE_AUGMENTATION_REGION,
    PROJECT_SOURCE_AUGMENTATION_CAPTURE, PROJECT_SOURCE_AUGMENTATION_SELECTION,
    PROJECT_SOURCE_AUGMENTATION_CHANGED } ProjectSourceAugmentationStatus;
/** Borrow O, F and original raw source through destruction. Own a new U capture
 * only if some preexisting F region is not covered by O. original/selected then
 * point to those owned common facts, otherwise to borrowed configured/raw facts.
 * No compiler analysis, source edits, whole-operation or freshness authority.
 * Already covered identities and O bytes still require the final shared complete
 * root proof, including formerly selected suppliers no longer selected by U.
 */
typedef struct {
    SourceArena arena;
    SourceRootCapture capture;
    SourceTreeSelection source;
    const SourceRootCapture *original;
    const SourceTreeSelection *selected;
    const char *failed_path;
    SourcePhysicalIssue physical_issue;
    SourceOperationRootsStatus region_status;
    ProjectSourceAugmentationStatus status;
    bool attempted, valid;
} ProjectSourceAugmentation;
/** Initialize independent metadata, capture and raw owners without I/O. */
void project_source_augmentation_init(ProjectSourceAugmentation *, SourceAllocator);
/** Retire raw borrowers before augmented catalog and owned descriptor rows. */
void project_source_augmentation_destroy(ProjectSourceAugmentation *);
/** Build once from genuine configured raw placement/F proposals. Add only
 * preexisting F regions not already covered by the exact O union, using the
 * shared symbolic boundary decision, then capture the entire union once. Never
 * change an already covered identity by substituting a final-only catalog.
 * Missing future roots add no fictitious original inputs. Reselect the original
 * raw source in U and require both physical membership inclusions, exact kinds,
 * endpoint inode/mode observations and bytes against O's raw capture; refuse changes,
 * never recapture fallback. Later domain admission and the complete owner still
 * prove selected-version, identity/material/root and graph requirements.
 * No mutation, compiler/Ama/native/test/CMake/network execution. Finite O/F
 * region scans plus one common capture and quadratic raw comparisons terminate;
 * O(O+F) descriptors plus child capture/raw/region scratch are owned. A failed
 * prefix remains destructible but unaccepted.
 */
ProjectSourceAugmentationStatus project_source_augmentation_build(ProjectSourceAugmentation *, const ProjectSourceRootSettings *);
#endif
