/** @file
 * @brief Separate original command acquisition, removal construction and final compiler sealing.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * gives no command-specific bypass around the shared complete owner's proofs.
 */
#ifndef AMA_PROJECT_SOURCE_PREPARATION_INTERNAL_H
#define AMA_PROJECT_SOURCE_PREPARATION_INTERNAL_H
#include "preparation.h"
/** Capture explicit configured roots and source, then write/check original bytes
 * in a protected workspace. The caller has admitted the immutable request. */
ProjectSourcePreparationStatus project_source_preparation_original(ProjectSourcePreparation *);
/** Normalize destination and prepare exact F choices and one common original U,
 * then reconstruct actual raw placement over that U without source mutation. */
ProjectSourcePreparationStatus project_source_preparation_destination(ProjectSourcePreparation *);
/** Join exact copy/move seeds, finite closure and designated version proposals.
 * Require completed original capture; this grants no final-plan authority. */
ProjectSourcePreparationStatus project_source_preparation_seed(ProjectSourcePreparation *);
/** Derive actual deletion material/settings, observe once and prepare the shared
 * complete owner. Borrow all exact source facts; never construct a copy closure. */
ProjectSourcePreparationStatus project_source_preparation_delete(ProjectSourcePreparation *);
/** Serialize/check the complete owner's exact final view and seal the same owner;
 * no module, test, native or CMake execution and no source mutation occur. */
ProjectSourcePreparationStatus project_source_preparation_seal(ProjectSourcePreparation *);
#endif
