/** @file
 * @brief Compare complete physical unit evidence without changing semantic selection.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires unchanged main/companion membership when preserving original or
 * previously ignored suppliers and structural parent providers.
 */
#ifndef AMA_SOURCE_CAPTURE_PRESERVATION_H
#define AMA_SOURCE_CAPTURE_PRESERVATION_H
#include "image.h"
/** Compare an actual captured unit's main and complete immediate dot companions
 * with one immutable image. Require exact paths, kinds, bytes and known complete
 * parent membership in both directions. Unknown evidence refuses; directory
 * subtrees and semantic selection remain separate obligations. No I/O/allocation
 * or mutation. For K unit members and B image entries, O(KB) comparisons and
 * constant scratch. Borrow inputs throughout the call.
 */
bool source_captured_unit_preserved(const SourceCapturedUnit *, const SourceCatalogImage *);
#endif
