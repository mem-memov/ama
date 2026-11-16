/** @file
 * @brief Prepare exact symbolic-link bytes without traversing or rewriting their targets.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * carries accompanying assets as opaque physical entries.
 */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../internal.h"

/** One extra byte detects truncation or an unexpectedly longer target without inferring any path semantics. Component contract: [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03). */
static bool verify(RelocationTransaction *transaction, const char *path, const SourceText *text) {
    size_t length = source_length(text);
    if (length == SIZE_MAX) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"A link target buffer size is unrepresentable",path); }
    unsigned char *bytes = source_arena_allocate(&transaction->arena,length+1);
    if (bytes == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Prepared link verification allocation failed",path); }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_VERIFY,path)) { return false; }
    ssize_t received = readlink(path,(char *)bytes,length+1);
    return (received >= 0 && (size_t)received == length && memcmp(bytes,source_bytes(text),length) == 0)
        || relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"Prepared link bytes differ from their captured target",path);
}
/** Linux link permissions are fixed; reject unsupported metadata instead of silently following a target with chmod. Component contract: [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03). */
bool relocation_transaction_link(RelocationTransaction *transaction, const char *path,
    const SourceText *text, const struct stat *expected, struct stat *prepared) {
    if ((expected->st_mode&07777) != 0777) {
        return relocation_transaction_fail(transaction,RELOCATION_UNSUPPORTED_PATH,"The captured symbolic-link permission model is unsupported",path);
    }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_LINK,path)
        || symlink((const char *)source_bytes(text),path) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private symbolic link could not be created",path);
    }
    if (!verify(transaction,path,text) || lstat(path,prepared) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private symbolic link could not be verified",path);
    }
    if ((prepared->st_uid != expected->st_uid || prepared->st_gid != expected->st_gid)
        && (!relocation_transaction_permit(transaction,RELOCATION_IO_OWNER,path)
            || lchown(path,expected->st_uid,expected->st_gid) != 0)) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Prepared link ownership could not be preserved",path);
    }
    struct timespec times[] = {expected->st_atim,expected->st_mtim};
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_TIMES,path)
        || utimensat(AT_FDCWD,path,times,AT_SYMLINK_NOFOLLOW) != 0 || lstat(path,prepared) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Prepared link timestamps could not be preserved",path);
    }
    return true;
}
