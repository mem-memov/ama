/** @file
 * @brief Separate finite root-setting allocation, placement provenance and explicit final scope application.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * does not let candidate settings replace final shared plan validation.
 */
#ifndef AMA_PROJECT_SOURCE_ROOT_SETTINGS_INTERNAL_H
#define AMA_PROJECT_SOURCE_ROOT_SETTINGS_INTERNAL_H
#include "roots.h"
/** Append or reuse an exact canonical final key. Equal keys require equal kind;
 * only an explicit destination choice may replace its complete exclusion scope. */
ProjectSourceRootSettingsStatus project_source_root_settings_add(ProjectSourceRootSettings *, ModuleRoot, bool explicit_choice, size_t *);
/** Derive O dispositions and actual raw directory carries without host reads. */
ProjectSourceRootSettingsStatus project_source_root_settings_originals(ProjectSourceRootSettings *, SourceOperationKind);
/** Normalize explicit destination roots and bind their complete final scopes. */
ProjectSourceRootSettingsStatus project_source_root_settings_explicit(ProjectSourceRootSettings *, const ProjectSourceRootRequest *, size_t);
#endif
