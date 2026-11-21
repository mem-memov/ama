/** @file
 * @brief Own explicit output-mode/component selection while borrowing one checked executable root.
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01)
 * requires a concrete selected root; [EMBED-09](../../../spec/implementation/c-library.md#embed-09)
 * distinguishes executable and source-inclusion delivery without changing stateful exports.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) separates copied
 * component bytes from the longer-lived checked frontend/IR that this model borrows.
 */
#ifndef AMA_DELIVERY_SELECTION_H
#define AMA_DELIVERY_SELECTION_H
#include "compiler/frontend.h"
#include "delivery/mode.h"
/** Explicit immutable caller selection. Strings survive synchronous preparation;
 * runtime argument values are separate under
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01).
 */
typedef struct { DeliveryMode mode; const char *component; } DeliveryRequest;
/** Arena-owned selection; frontend/IR/source owners outlive consumption. Component
 * bytes are copied unchanged, including spaces and Unicode. This metadata is
 * not itself a launcher/decoder implementation under
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct {
    DeliveryMode mode;
    const char *component;
    const CompilerFrontend *frontend;
    const PlannedType *root;
} DeliverySelection;
/** Prepare one explicit closed selection without output, C compilation or execution.
 * Arena/request/output/error are valid; frontend is initialized. Failure preserves
 * output and arena owns failed scratch. Reject invalid/authoring/nonexecutable
 * frontend output and invalid UTF-8/empty components. Borrow expiry is the C
 * caller's lifetime responsibility under
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01) and
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
bool delivery_selection_prepare(SourceArena *, const CompilerFrontend *, const DeliveryRequest *,
    const DeliverySelection **output, Diagnostic *error);
#endif
