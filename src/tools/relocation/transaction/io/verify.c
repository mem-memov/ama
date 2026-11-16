/** @file
 * @brief Verify exact prepared file bytes with bounded stack storage and no checksum assumptions.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * verifies prepared replacements before originals can be retired.
 */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "../internal.h"
/** A successful prefix strictly advances; EOF must occur exactly at the expected byte count. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_verify(RelocationTransaction *transaction, int descriptor, const char *path, const SourceText *expected) {
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_VERIFY,path) || lseek(descriptor,0,SEEK_SET) < 0) {
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Prepared bytes could not be positioned for verification",path);
    }
    unsigned char buffer[4096];
    size_t offset = 0, length = source_length(expected);
    for (;;) {
        size_t count = length-offset;
        if (count > sizeof(buffer)) { count = sizeof(buffer); }
        if (count == 0) { count = 1; }
        ssize_t received = read(descriptor,buffer,count);
        if (received < 0 && errno == EINTR) { continue; }
        if (received < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Prepared bytes could not be verified",path); }
        if (received == 0) {
            return offset == length || relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"Prepared bytes ended before their captured length",path);
        }
        if ((size_t)received > length-offset || memcmp(buffer,source_bytes(expected)+offset,(size_t)received) != 0) {
            return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"Prepared bytes differ from their validated snapshot",path);
        }
        offset += (size_t)received;
    }
}
