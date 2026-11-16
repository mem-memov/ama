/** @file
 * @brief Capture symbolic-link text as opaque bytes without traversing its target.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * carries accompanying files without rewriting embedded physical paths.
 */
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include "../internal.h"
/** A full buffer may mean truncation; geometric growth terminates for every stable finite link target. */
bool relocation_staging_link(RelocationStaging *staging, const char *path, const SourceText **output) {
    for (size_t capacity = 64;;) {
        if (capacity > (size_t)SSIZE_MAX) {
            return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Link target exceeds the read interface range",(SourceSpan){0});
        }
        unsigned char *bytes = source_arena_allocate(&staging->arena,capacity);
        if (bytes == NULL) {
            return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Link target allocation failed",(SourceSpan){0});
        }
        ssize_t length = readlink(path,(char *)bytes,capacity);
        if (length < 0) { return relocation_staging_fail(staging,RELOCATION_IO_ERROR,"Link target could not be read",(SourceSpan){0}); }
        if ((size_t)length < capacity) {
            if (source_manager_add(&staging->sources,path,bytes,(size_t)length,output) != SOURCE_OK) {
                return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Link target ownership allocation failed",(SourceSpan){0});
            }
            return true;
        }
        if (capacity > SIZE_MAX/2) {
            return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Link target size is unrepresentable",(SourceSpan){0});
        }
        capacity *= 2;
    }
}
