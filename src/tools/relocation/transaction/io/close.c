/** @file
 * @brief Retire a transaction descriptor once even when close or a simulated close acknowledgement fails.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps failed preparation/cleanup resource ownership explicit.
 */
#include <unistd.h>
#include "../internal.h"
/** The checkpoint follows the sole close attempt, so injected refusal never leaves an unowned live descriptor. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_close(RelocationTransaction *transaction, int descriptor, const char *path) {
    int result = close(descriptor);
    bool admitted = relocation_transaction_permit(transaction,RELOCATION_IO_CLOSE,path);
    return (result == 0 && admitted)
        || relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A relocation descriptor close failed",path);
}
