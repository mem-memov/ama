/** @file
 * @brief Acquire descriptor-backed exclusive directory leases without creating lock files in source trees.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires exclusive project state; retained directory identities survive path renames.
 */
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include "../internal.h"

/** Physical identity deduplication prevents self-conflict when roots and endpoint parents share a directory. */
bool relocation_transaction_lease_directory(RelocationTransaction *transaction, const char *path) {
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_LEASE,path)) { return false; }
    int descriptor = open(path,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A relocation lease directory could not be opened",path); }
    struct stat metadata;
    if (fstat(descriptor,&metadata) != 0) {
        (void)close(descriptor);
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A relocation lease identity could not be read",path);
    }
    for (const RelocationDirectoryLease *other = transaction->leases; other != NULL; other = other->next) {
        if (other->device == metadata.st_dev && other->inode == metadata.st_ino) {
            (void)close(descriptor);
            return true;
        }
    }
    if (flock(descriptor,LOCK_EX|LOCK_NB) != 0) {
        int failure = errno;
        (void)close(descriptor);
        return relocation_transaction_fail(transaction,failure == EWOULDBLOCK || failure == EAGAIN
            ? RELOCATION_BUSY : RELOCATION_IO_ERROR,"Exclusive relocation access is unavailable",path);
    }
    RelocationDirectoryLease *lease = source_arena_allocate(&transaction->arena,sizeof(*lease));
    if (lease == NULL) {
        (void)close(descriptor);
        return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Relocation lease ownership allocation failed",path);
    }
    *lease = (RelocationDirectoryLease){.next=transaction->leases,.descriptor=descriptor,
        .device=metadata.st_dev,.inode=metadata.st_ino,.path=path};
    transaction->leases = lease;
    return true;
}
/** Linux closes a valid descriptor even on an interrupted close; each owned descriptor receives exactly one attempt. */
void relocation_transaction_release(RelocationTransaction *transaction) {
    for (RelocationDirectoryLease *lease = transaction->leases; lease != NULL; lease = lease->next) {
        if (lease->descriptor >= 0) {
            (void)close(lease->descriptor);
            lease->descriptor = -1;
        }
    }
}
