/** @file
 * @brief Write and verify captured replacement bytes only in an exclusively created private file.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains originals until every replacement is prepared and checked.
 */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include "../internal.h"

/** Verification precedes final timestamps so read-induced access-time updates do not undo attribute preparation. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_file(RelocationTransaction *transaction, const char *path,
    const SourceText *text, const struct stat *expected, struct stat *prepared) {
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_FILE,path)) { return false; }
    int descriptor = open(path,O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private prepared file could not be created",path); }
    bool complete = relocation_transaction_write(transaction,descriptor,path,source_bytes(text),source_length(text))
        && relocation_transaction_verify(transaction,descriptor,path,text)
        && relocation_transaction_attributes(transaction,descriptor,path,expected);
    if (complete && fstat(descriptor,prepared) != 0) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A prepared file identity could not be captured",path);
    }
    bool closed = relocation_transaction_close(transaction,descriptor,path);
    return complete && closed;
}
