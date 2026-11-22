/** @file
 * @brief Re-express an explicitly proven finite dependency-root set as concrete INI selectors.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids widening wildcard meaning; preparation never decides permission policy.
 */
#ifndef AMA_SOURCE_COPY_SELECTOR_REEXPRESSION_H
#define AMA_SOURCE_COPY_SELECTOR_REEXPRESSION_H
#include "../../edits/edits.h"
#include "source/configuration/document/document.h"
#include "source/configuration/selection/selection.h"
/** Prepare only selector-span replacements or a parsed-section insertion.
 * Require the valid package document of the edit owner's exact original source
 * and entries borrowed from one successful complete final view. Input duplicates
 * are idempotent; emit in view order. No wildcard, alias or repository copying.
 * Empty sets remove original selector tokens; unrelated bytes remain unchanged.
 * Existing line endings are retained; new separators use the first original
 * newline spelling (LF in an empty/unterminated document). Preserve nonempty
 * input's final-newline presence. A new document ends in LF.
 *
 * The caller must prove this requested set is exactly the mapped original
 * selection plus explicitly authorized additions. Reparse and compare final
 * actual selection/coverage and all reference permissions before publication.
 * This byte preparation alone proves neither those laws nor source-operation
 * acceptance. No I/O/mutation; finite view/root/selector/line scans. Scratch and
 * failed prefixes stay edit-arena-owned; failure poisons application. Invalid
 * closed/failed owners refuse unchanged. Return the exact edit-layer status.
 */
SourceEditsStatus source_copy_reexpress_selectors(SourceTextEdits *, const ConfigurationDocument *,
    const DependencyView *, const DependencyViewEntry *const *, size_t);
#endif
