/** @file
 * @brief Share exact context-parent topology queries across omission and copy-necessity proofs.
 * [SRC-05](../../../../../spec/language/source-and-names.md#src-05) requires
 * both physical plus storage and canonical logical ancestry.
 */
#ifndef AMA_SOURCE_COPY_CONTEXT_INTERNAL_H
#define AMA_SOURCE_COPY_CONTEXT_INTERNAL_H
#include "omissions.h"
#include "ancestors.h"
/** Borrow an exact canonical captured unit, or NULL. */
const SourceCapturedUnit *source_copy_context_unit(const SourceCapturedUnit *, const char *);
/** Borrow an original-key or final-key C row, or NULL. */
const SourceCloneMapping *source_copy_context_clone(const SourceCloneMap *, const char *, bool);
/** Require strict logical and physical plus ancestry; no basename approximation. */
bool source_copy_context_parent(const ModuleSource *, const ModuleSource *);
#endif
