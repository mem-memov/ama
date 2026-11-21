/** @file
 * @brief Retain exact declaration-order launch inputs without requiring unused transports.
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01)
 * preserves declared names; [CLI-02](../../../spec/implementation/launch-and-embedding.md#cli-02)
 * permits omitted complex inputs and derives text support from the actual literal capability.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) keeps descriptors
 * arena-owned while their checked frontend is borrowed;
 * [CLI-03](../../../spec/implementation/launch-and-embedding.md#cli-03) forbids
 * inventing a binary adapter from an input's shape.
 */
#ifndef AMA_DELIVERY_INPUTS_H
#define AMA_DELIVERY_INPUTS_H
#include "delivery/selection/selection.h"
/** Borrowed exact input declaration plus its parameter ordinal and text capability.
 * No name rewriting, requiredness or binary decoder is inferred under
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01) and
 * [CLI-03](../../../spec/implementation/launch-and-embedding.md#cli-03).
 */
typedef struct {
    const SemanticSymbol *field;
    size_t ordinal;
    bool text_supported;
} DeliveryInput;
/** Arena-owned immutable input table borrowing selection and its valid frontend.
 * Empty tables have NULL entries. Borrowed owners outlive consumption under
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct {
    const DeliverySelection *selection;
    const DeliveryInput *entries;
    size_t count;
} DeliveryInputs;
/** Capture every actual parameter in declaration order, including default-only
 * reference/list/compound inputs, and no result/local declaration. Require the
 * selection's still-valid unchanged frontend entry. Failure preserves output;
 * scratch remains arena-owned. No I/O, constructor or compiler process runs.
 * [CLI-02](../../../spec/implementation/launch-and-embedding.md#cli-02)
 * requires text capability without an extra copy/condition requirement.
 */
bool delivery_inputs_prepare(SourceArena *, const DeliverySelection *, const DeliveryInputs **, Diagnostic *);
#endif
