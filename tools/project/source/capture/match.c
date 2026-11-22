/** @file
 * @brief Compare retained unit observations without substituting newer source bytes.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects changed inputs; [PKG-07](../../../../spec/project/packages.md#pkg-07)
 * includes every same-module companion, even unknown extensions.
 */
#include <string.h>
#include "match.h"
/** Compare exact byte observations rather than allocator/source identities. */
static bool equal(const AcquisitionUnitEntry *left, const AcquisitionUnitEntry *right) {
    return left && right && left->directory == right->directory && !left->directory
        && source_length(left->file.content) == source_length(right->file.content)
        && !memcmp(source_bytes(left->file.content), source_bytes(right->file.content), source_length(left->file.content));
}
/** Require both directions of a same-module file set, including unknown suffixes. */
bool source_captured_unit_matches(const SourceCapturedUnit *unit, const AcquisitionUnitEntry *files) {
    const char *path = source_path(unit->source->text);
    size_t length = strlen(path);
    for (const AcquisitionUnitEntry *old = unit->files; old; old = old->next) {
        const AcquisitionUnitEntry *now = files;
        while (now && strcmp(old->file.path, now->file.path)) { now = now->next; }
        if (!equal(old, now)) { return false; }
    }
    for (const AcquisitionUnitEntry *now = files; now; now = now->next) {
        if (strncmp(now->file.path, path, length) || now->file.path[length] != '.' || !now->file.path[length + 1]) { continue; }
        const AcquisitionUnitEntry *old = unit->files;
        while (old && strcmp(old->file.path, now->file.path)) { old = old->next; }
        if (!equal(old, now)) { return false; }
    }
    /* Indexed mains retain original inode identity in addition to exact bytes.
     * Final whole-domain freshness still checks ancestor/directory membership. */
    struct stat metadata;
    return lstat(path, &metadata) == 0 && S_ISREG(metadata.st_mode)
        && metadata.st_dev == unit->source->device && metadata.st_ino == unit->source->inode;
}
