/** @file
 * @brief Separate whole-graph root preparation from actual-context requirement publication.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) keeps validation roots outside value-read paths.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial graph preparation.
 */
#ifndef AMA_STATIC_RESULT_GRAPHS_INTERNAL_H
#define AMA_STATIC_RESULT_GRAPHS_INTERNAL_H
#include "../internal.h"

/** Append one prepared actual root; caller supplies exactly one alternative.
 * The unpublished graph and partial work remain arena-owned on refusal.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains every local obligation.
 */
bool static_result_graph_work(StaticResultPlan *, StaticResultGraph *, StaticResultUse *, StaticResultDefinitionUse *);
/** Prepare all public definitions and producer statements in this exact context.
 * No root is made a value dependency of another root and no body is evaluated.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) distinguishes equality and definition work.
 */
bool static_result_graph_prepare(StaticResultPlan *, StaticResultGraph *);
#endif
