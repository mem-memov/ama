/** @file
 * @brief Prepare exact immutable root settings and the complete active-module analysis domain.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) makes source closure and settings explicit.
 */
#ifndef AMA_EDITOR_WORKSPACE_INTERNAL_H
#define AMA_EDITOR_WORKSPACE_INTERNAL_H
#include "../analysis/internal.h"
/** Deep-copy root settings and entry so later candidate validation cannot borrow transient caller options. */
bool editor_workspace_configuration(EditorAnalysis *, const ModuleRoot *, size_t, const char *);
/** Classify all catalog sources through the production layout/activation parser and return sorted active paths. */
bool editor_workspace_active(EditorAnalysis *, const char *const **, size_t *);
/** Finish a prepared catalog through the ordinary frontend and exact occurrence index. */
bool editor_workspace_finish(EditorAnalysis *);
#endif
