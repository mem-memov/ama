/** @file
 * @brief Share original child-operator caches and source-only equation operations.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) connects all
 * structural operands; [TPL-11](../../../../../../spec/language/templates.md#tpl-11)
 * gives these facts no actual context or validation certificate.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) retains
 * failed partial caches in the publication arena until normal teardown.
 */
#ifndef AMA_STATIC_ORDINARY_INPUTS_INTERNAL_H
#define AMA_STATIC_ORDINARY_INPUTS_INTERNAL_H
#include "../internal.h"

/** One publication-owned original input operator, borrowing its longer-lived
 * result source and immutable operand map. Application snapshots may refresh
 * only when independent nominal evidence resolves their head under
 * [TPL-10](../../../../../../spec/language/templates.md#tpl-10).
 * Selected operands additionally retain their attributed supplier proofs;
 * neither form provides a using-module instance or ordinary body certificate
 * under [TPL-11](../../../../../../spec/language/templates.md#tpl-11).
 */
typedef struct StaticOrdinaryOperand {
    struct StaticOrdinaryOperand *next;
    const StaticResultSource *source;
    const StaticTermApplication *prepared;
    const StaticApplicationRecord *application;
    struct StaticOrdinaryProjection *projection;
} StaticOrdinaryOperand;

/** Collect a validated ordinary input forest once in original preorder.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) retains the
 * exact annotation environment and child ownership. The caller supplies a
 * registered ordinary annotation. Failure leaves partial arena-owned records
 * destructible and never publishes the collected flag.
 */
bool static_ordinary_collect_inputs(StaticPublicationPlan *, StaticOrdinaryType *);

/** Apply existing pure kernels to every retained original child operator.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) contributes
 * structural equations; [TPL-11](../../../../../../spec/language/templates.md#tpl-11)
 * reserves actual creation and validation for guarded uses. Failure preserves
 * the dependency owner's diagnostic and normal publication teardown.
 */
bool static_ordinary_input_equations(StaticPublicationPlan *, StaticOrdinaryType *);
#endif
