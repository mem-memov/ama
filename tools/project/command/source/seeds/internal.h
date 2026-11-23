/** @file
 * @brief Separate command version authority derivation from closure/seed orchestration.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires designated storage to agree with actual explicit placement.
 */
#ifndef AMA_PROJECT_SOURCE_SEEDS_INTERNAL_H
#define AMA_PROJECT_SOURCE_SEEDS_INTERNAL_H
#include "seeds.h"
/** Derive all ordinary explicit version authorities; caller owns admitted seeds.
 * No I/O or final selection claim; failed scratch stays in the result arena. */
ProjectSourceSeedsStatus project_source_seeds_versions(ProjectSourceSeeds *);
/** Derive one exact ordinary version proposal from an admitted explicit main.
 * Allocate only owned strings; output changes on success. No host reads. */
ProjectSourceSeedsStatus project_source_seeds_version(ProjectSourceSeeds *, const SourceCloneMapping *, SourceVersionPlacement *);
#endif
