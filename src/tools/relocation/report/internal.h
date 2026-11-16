/** @file
 * @brief Separate owned settings, module identity rows and physical report roles.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete reporting without borrowing destroyed compiler snapshots.
 */
#ifndef AMA_RELOCATION_REPORT_INTERNAL_H
#define AMA_RELOCATION_REPORT_INTERNAL_H
#include "report.h"
/** Record the first failed owned copy without allocating a diagnostic. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_report_memory(RelocationReport *);
/** Copy explicit before/after root kind/path/exclusion arrays and entry identity. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_report_settings(RelocationReport *, const RelocationStaging *);
/** Copy every source placement and logical identity mapping, including inactive drafts and newly covered sources. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_report_modules(RelocationReport *, const RelocationStaging *);
/** Copy physical entries and explicitly classified rewrite/draft/invalidation roles. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_report_files(RelocationReport *, const RelocationStaging *);
#endif
