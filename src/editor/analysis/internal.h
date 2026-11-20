/** @file
 * @brief Share exact checked occurrence construction without exposing partially built indexes.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) retains failed allocations until owner teardown.
 */
#ifndef AMA_EDITOR_INTERNAL_H
#define AMA_EDITOR_INTERNAL_H
#include "analysis.h"
/** Add/deduplicate one exact spelling; conflicting metadata fails instead of selecting a guess. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
bool editor_index_add(EditorAnalysis *, EditorOccurrence);
/** Index accepted graph/module/formal identities directly from the compiler's
 * source inventory, without substituting returned Types for written paths.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) keeps source producers separate from ordinary values.
 */
bool editor_index_static(EditorAnalysis *);
/** Retain all actual foreign formal origins from accepted normalized static
 * applications; signature-only unknown origins do not acquire invented targets.
 * [TPL-07](../../../spec/language/templates.md#tpl-07) preserves origin identity across structurally compatible signatures.
 */
bool editor_index_static_labels(EditorAnalysis *);
/** Index every accepted lexical field/use, including uninstantiated ordinary
 * templates, from the compiler's original source role inventory.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) fixes names before substitution.
 */
bool editor_index_runtime_source(EditorAnalysis *);
/** Stably order and deduplicate exact records, rejecting any contradictory resolved identity. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
bool editor_index_order(EditorAnalysis *);
/** Index the fully validated semantic graph only; failure never publishes indexed=true. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
bool editor_index_build(EditorAnalysis *);
/** Index exact resolved source/access views, including nested index operands and qualified constants. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
bool editor_index_access(EditorAnalysis *, const SemanticAccess *);
/** Index source spellings using their resolved nominal/constant/access identities. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
bool editor_index_source(EditorAnalysis *, const SemanticSource *);
#endif
