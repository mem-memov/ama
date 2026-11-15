/** @file
 * @brief Share exact checked occurrence construction without exposing partially built indexes.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) retains failed allocations until owner teardown.
 */
#ifndef AMA_EDITOR_INTERNAL_H
#define AMA_EDITOR_INTERNAL_H
#include "analysis.h"
/** Add/deduplicate one exact spelling; conflicting metadata fails instead of selecting a guess. */
bool editor_index_add(EditorAnalysis *, SourceSpan, const SemanticSymbol *, const SemanticType *, bool);
/** Stably order and deduplicate exact records, rejecting any contradictory resolved identity. */
bool editor_index_order(EditorAnalysis *);
/** Index the fully validated semantic graph only; failure never publishes indexed=true. */
bool editor_index_build(EditorAnalysis *);
/** Index exact resolved source/access views, including nested index operands and qualified constants. */
bool editor_index_access(EditorAnalysis *, const SemanticAccess *);
/** Index source spellings using their resolved nominal/constant/access identities. */
bool editor_index_source(EditorAnalysis *, const SemanticSource *);
#endif
