/** @file
 * @brief Synchronize prepared directory entries separately from their contained file data.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains complete prepared replacements before any original can be retired.
 */
#include <fcntl.h>
#include <unistd.h>
#include "../internal.h"
/** A private tree's root sync does not itself flush its name in the containing slot directory. */
bool relocation_transaction_sync(RelocationTransaction *transaction, const char *path) {
    int descriptor = open(path,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A relocation synchronization directory could not be opened",path); }
    bool complete = relocation_transaction_permit(transaction,RELOCATION_IO_SYNC,path) && fsync(descriptor) == 0;
    if (!complete) { relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Relocation directory entries could not be synchronized",path); }
    bool closed = relocation_transaction_close(transaction,descriptor,path);
    return complete && closed;
}
