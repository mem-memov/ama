/** @file
 * @brief Separate raw tree reading from exact original-module projection.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * preserves physical selection, including opaque material and drafts.
 */
#ifndef AMA_PROJECT_SOURCE_SELECTION_INTERNAL_H
#define AMA_PROJECT_SOURCE_SELECTION_INTERNAL_H
#include "tree.h"
/** Require an exact configured physical root ancestor without namespace inference.
 * Inputs are nonempty resolved physical keys; a filesystem root contains every
 * same-prefix absolute descendant, while ordinary names require a slash boundary.
 */
bool source_tree_inside(const char *, const char *);
/** Project every covered indexed unit only after checking its original observations. */
SourceTreeStatus source_tree_project(SourceTreeSelection *, const SourceRootCapture *);
#endif
