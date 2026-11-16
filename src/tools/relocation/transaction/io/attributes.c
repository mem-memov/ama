/** @file
 * @brief Preserve captured ownership, permissions and timestamps only on private prepared objects.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * carries source/native/assets intact; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * leaves originals untouched during preparation.
 */
#include <unistd.h>
#include "../internal.h"
/** Ownership precedes mode because chown may clear special permission bits; all metadata precedes synchronization. Component contract: [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03), [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_attributes(RelocationTransaction *transaction, int descriptor, const char *path, const struct stat *expected) {
    struct stat current;
    if (fstat(descriptor,&current) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A prepared object identity could not be read",path);
    }
    if ((current.st_uid != expected->st_uid || current.st_gid != expected->st_gid)
        && (!relocation_transaction_permit(transaction,RELOCATION_IO_OWNER,path)
            || fchown(descriptor,expected->st_uid,expected->st_gid) != 0)) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Prepared ownership could not be preserved",path);
    }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_MODE,path) || fchmod(descriptor,expected->st_mode&07777) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Prepared permissions could not be preserved",path);
    }
    struct timespec times[] = {expected->st_atim,expected->st_mtim};
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_TIMES,path) || futimens(descriptor,times) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Prepared timestamps could not be preserved",path);
    }
    return (relocation_transaction_permit(transaction,RELOCATION_IO_SYNC,path) && fsync(descriptor) == 0)
        || relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Prepared data could not be synchronized",path);
}
