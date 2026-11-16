/** @file
 * @brief Confirm a read commit or terminal receipt durably before retiring originals or acknowledging historical completion.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * never assumes that an earlier executor successfully synchronized a complete marker merely because its bytes are readable.
 */
#include <fcntl.h>
#include <unistd.h>
#include "internal.h"

/** Read-only record synchronization preserves its bytes and never visits former public project paths. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_recovery_acknowledge(RelocationRecovery *recovery) {
    RelocationTransaction *transaction = &recovery->transaction;
    const char *path = recovery->preparing ? transaction->intent_record : transaction->journal_record;
    int descriptor = open(path,O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A retained outcome record could not be reopened for acknowledgement",path);
    }
    bool complete = relocation_transaction_permit(transaction,RELOCATION_IO_SYNC,path);
    if (complete && fsync(descriptor) != 0) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A retained outcome record could not be synchronized",path);
    }
    bool closed = relocation_transaction_close(transaction,descriptor,path);
    return complete && closed && relocation_transaction_sync(transaction,transaction->journal);
}
