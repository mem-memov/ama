/** @file
 * @brief Separate original root-region selection, raw consistency checks and capture orchestration.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires one common original domain, never independently merged analyses.
 */
#ifndef AMA_PROJECT_SOURCE_AUGMENTATION_INTERNAL_H
#define AMA_PROJECT_SOURCE_AUGMENTATION_INTERNAL_H
#include "augmentation.h"
/** Values/merged exclusion pointers belong to owner arena; strings retain O/F
 * owners. expanded also records same-path exclusion changes without count growth. */
typedef struct { ModuleRoot *values; size_t count; bool expanded; } ProjectSourceAugmentationRoots;
/** Select uncovered existing regions and exactly merge equal-root scopes. */
ProjectSourceAugmentationStatus project_source_augmentation_roots(ProjectSourceAugmentation *, const ProjectSourceRootSettings *, ProjectSourceAugmentationRoots *);
/** Read source from the newly captured U once and compare original raw facts. */
ProjectSourceAugmentationStatus project_source_augmentation_source(ProjectSourceAugmentation *, const ProjectSourceRootSettings *);
#endif
