/** @file
 * @brief Make transaction filesystem phase refusals deterministic without replacing real filesystem composition.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires failure injection over publication and inverse operations.
 */
#include "../internal.h"
/** Tests may refuse the next actual operation; normal application uses the same code with no callback. */
bool relocation_transaction_permit(RelocationTransaction *transaction, RelocationIOOperation operation, const char *path) {
    return transaction->io.permit == NULL || transaction->io.permit(transaction->io.context,operation,path)
        || relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A relocation filesystem operation was refused",path);
}
