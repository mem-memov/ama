/** @file
 * @brief Derive exact observation demands and removal keys from one original selection.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * binds physical deletion to complete selected original material;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates read-only preparation from observation, review and publication.
 */
#ifndef AMA_SOURCE_REMOVAL_SELECTION_MATERIAL_H
#define AMA_SOURCE_REMOVAL_SELECTION_MATERIAL_H
#include "selection.h"
#include "../../observation/observation.h"
#include "../../image/projection.h"
/** Borrow exact selected keys; the caller arena owns both equal-length arrays. */
typedef struct {
    const SourceObservationRequest *requests;
    const SourceImageChange *changes;
    size_t count;
} SourceRemovalMaterial;
/** Invalid owners/counts are distinct from bounded allocation refusal. */
typedef enum { SOURCE_REMOVAL_MATERIAL_OK, SOURCE_REMOVAL_MATERIAL_INVALID,
    SOURCE_REMOVAL_MATERIAL_MEMORY } SourceRemovalMaterialStatus;
/** Prepare one observation demand and REMOVE per unique selected physical member.
 * Directory demands require complete children; regular demands require content.
 * Output changes only on complete success, including the allocation-free empty
 * selection. Failure scratch remains arena-owned. Selection and all captured
 * keys outlive consumers. O(F) finite work and storage with checked size bounds.
 * No I/O or permission is granted: the operation plan must compare observed
 * material, review the complete original cut and prove the final image.
 */
SourceRemovalMaterialStatus source_removal_material_prepare(SourceArena *,
    const SourceRemovalSelection *, SourceRemovalMaterial *);
#endif
