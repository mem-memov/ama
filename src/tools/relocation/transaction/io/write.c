/** @file
 * @brief Write complete prepared data/journal records without accepting zero-progress or partial success.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete replacement contents before originals can be retired.
 */
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include "../internal.h"
/** Short writes advance exactly; zero progress and actual errors reject without publishing a source change. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_write(RelocationTransaction *transaction, int descriptor, const char *path, const unsigned char *data, size_t length) {
    size_t offset = 0;
    while (offset < length) {
        size_t count = length-offset;
        if (count > (size_t)SSIZE_MAX) { count = (size_t)SSIZE_MAX; }
        if (!relocation_transaction_permit(transaction,RELOCATION_IO_WRITE,path)) { return false; }
        ssize_t written = write(descriptor,data+offset,count);
        if (written < 0 && errno == EINTR) { continue; }
        if (written <= 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Prepared bytes could not be written",path); }
        offset += (size_t)written;
    }
    return true;
}
