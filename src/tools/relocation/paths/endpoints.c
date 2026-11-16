/** @file
 * @brief Reject unsafe directory endpoints before any relocation discovery or mutation.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * rejects merging, descendant moves and unavailable required parent writes.
 */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "../internal.h"

/** Probe the actual parent access required for rename/unlink or destination creation. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01). */
static bool writable_parent(RelocationPreview *preview, const char *path) {
    char *parent = source_arena_copy(&preview->arena,path);
    if (parent == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,path,NULL); }
    char *separator = strrchr(parent,'/');
    if (separator == parent) { separator[1] = 0; }
    else { *separator = 0; }
    return access(parent,W_OK|X_OK) == 0 || relocation_fail(preview,RELOCATION_READ_ONLY,parent,NULL);
}

/** Canonical physical endpoints make containment exact; existing D is always rejected, including S itself. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01). */
bool relocation_endpoints(RelocationPreview *preview, const RelocationRequest *request) {
    preview->source = relocation_path(preview,request->source,false);
    if (preview->source == NULL) { return false; }
    preview->destination = relocation_path(preview,request->destination,true);
    if (preview->destination == NULL) { return false; }
    struct stat metadata;
    if (lstat(preview->destination,&metadata) == 0) {
        return relocation_fail(preview,RELOCATION_DESTINATION_EXISTS,preview->destination,NULL);
    }
    if (errno != ENOENT) { return relocation_fail(preview,RELOCATION_IO_ERROR,preview->destination,NULL); }
    if (relocation_contains(preview->source,preview->destination)) {
        return relocation_fail(preview,RELOCATION_DESCENDANT,preview->destination,preview->source);
    }
    return writable_parent(preview,preview->source) && writable_parent(preview,preview->destination);
}
