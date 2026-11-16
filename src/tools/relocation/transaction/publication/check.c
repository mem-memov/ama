/** @file
 * @brief Check original state before retirement and distinguish absent, owned and foreign paths without guessing side effects.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects changed inputs and preserves saved originals after uncertain filesystem outcomes.
 */
#include <errno.h>
#include <fcntl.h>
#include "../internal.h"
#include "../../staging/internal.h"

/** Device/inode/type identify an owned renamed object; rename-induced change times are not mistaken for replacement. */
RelocationPathState relocation_transaction_probe(RelocationTransaction *transaction, const char *path, const struct stat *expected) {
    struct stat metadata;
    if (lstat(path,&metadata) != 0) {
        if (errno == ENOENT) { return RELOCATION_PATH_ABSENT; }
        relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A relocation path state could not be inspected",path);
        return RELOCATION_PATH_ERROR;
    }
    if (expected != NULL && metadata.st_dev == expected->st_dev && metadata.st_ino == expected->st_ino
        && (metadata.st_mode&S_IFMT) == (expected->st_mode&S_IFMT)) { return RELOCATION_PATH_EXPECTED; }
    return RELOCATION_PATH_OTHER;
}
/** Original regular-file verification uses no allocation; a changed source is never backed up as if it were the planned original. */
bool relocation_transaction_original(RelocationTransaction *transaction, const RelocationTransactionAction *action) {
    struct stat metadata;
    if (lstat(action->path,&metadata) != 0 || !relocation_staging_metadata(&metadata,&action->original_metadata)) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"An original relocation target changed before replacement",action->path);
    }
    if (action->file == NULL || action->file->original == NULL || !S_ISREG(metadata.st_mode)) { return true; }
    int descriptor = open(action->path,O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"An original replacement input could not be opened",action->path); }
    bool complete = relocation_transaction_verify(transaction,descriptor,action->path,action->file->original);
    bool closed = relocation_transaction_close(transaction,descriptor,action->path);
    return complete && closed;
}
