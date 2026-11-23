/** @file
 * @brief Match actual captured module ancestors by nearest-first structural position.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * makes retained and copied parents occupy positions alike;
 * [SRC-05](../../../../../spec/language/source-and-names.md#src-05) distinguishes
 * module plus storage from ordinary containing directories. These queries share
 * the command planner's position model with the final context certificate.
 */
#ifndef AMA_SOURCE_COPY_CONTEXT_ANCESTORS_H
#define AMA_SOURCE_COPY_CONTEXT_ANCESTORS_H
#include "../../capture/capture.h"
/** Return a strict ancestor's zero-based nearest-first position in a genuine
 * finite captured chain. Both source handles must belong to units exactly;
 * equal names/bytes on foreign handles grant no membership. Require physical
 * plus and logical ancestry. Output changes only on success. No I/O, allocation
 * or mutation; O(U) exact topology comparisons and constant scratch. */
bool source_copy_context_position(const SourceCapturedUnit *, const ModuleSource *, const ModuleSource *, size_t *);
/** Borrow the unique ancestor at position, or NULL when absent/foreign/invalid.
 * Input units must be one valid immutable captured list. Ordinary directories
 * count zero; no iteration fallback chooses another ancestor. No allocation,
 * I/O or mutation. O(U squared) topology comparisons, constant scratch. */
const SourceCapturedUnit *source_copy_context_ancestor(const SourceCapturedUnit *, const ModuleSource *, size_t);
#endif
