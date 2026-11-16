/** @file
 * @brief Finalize prepared directory metadata after all of its child entries exist.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * preserves the selected physical layout without modifying original directories.
 */
#include <fcntl.h>
#include "../internal.h"
/** Call bottom-up after child creation so directory mode/timestamps are final when its descriptor is synchronized. */
bool relocation_transaction_directory(RelocationTransaction *transaction, const char *path,
    const struct stat *expected, struct stat *prepared) {
    int descriptor = open(path,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if (descriptor < 0) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A private prepared directory could not be opened",path); }
    bool complete = relocation_transaction_attributes(transaction,descriptor,path,expected);
    if (complete && fstat(descriptor,prepared) != 0) {
        complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A prepared directory identity could not be captured",path);
    }
    bool closed = relocation_transaction_close(transaction,descriptor,path);
    return complete && closed;
}
