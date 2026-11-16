/** @file
 * @brief Compose real label dispatch, ABI gates, lifecycle and typed transport once.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) binds
 * exported factories to exact descriptors, independently of the launcher root.
 */
#include <stdlib.h>
#include <string.h>
#include "assembly.h"
#include "library/transport/codecs.h"
/** Include paths reject directive-breaking bytes; backslashes are platform separators only on other hosts. Component contract: [EMBED-01](../../../spec/implementation/c-library.md#embed-01). */
static bool include_path(const char *path) {
    if (path == NULL || path[0] == '\0') { return false; }
    for (const unsigned char *byte=(const unsigned char *)path; *byte!=0; byte++) {
        if (*byte=='"' || *byte=='\\' || *byte<32 || *byte==127) { return false; }
        /* Translation phase 1 precedes header-name recognition in C11. */
        if (byte[0]=='?' && byte[1]=='?' && byte[2]!=0
            && strchr("=/'()!<>-",byte[2])!=NULL) { return false; }
    }
    return true;
}
bool library_assembly_write(const LibraryBuild *build, const NativeScaffold *scaffold,
    const char *header, const char *project, FILE *output) {
    if (!build->valid || scaffold->frontend != &build->frontend || !scaffold->ready
        || !include_path(header) || !include_path(project)
        || !library_header_target(build, NULL)
        || build->export_count > SIZE_MAX / sizeof(uint64_t)) { return false; }
    uint64_t *types = malloc(build->export_count * sizeof(*types));
    if (types == NULL) { return false; }
    size_t count = 0;
    LibraryCodecs codecs = {.assembly=scaffold};
    for (const LibraryExport *exported = build->exports; exported != NULL; exported = exported->next) {
        size_t ordinal = library_codec_type_id(&codecs, exported->type);
        if (ordinal == 0) { free(types); return false; }
        types[count++] = ordinal;
    }
    bool written = fprintf(output, "#include \"%s\"\n#include \"%s\"\n", header, project) >= 0
        && library_abi_write(build, output)
        && library_lifecycle_write(build, types, count, output)
        && library_snapshots_write(build, output)
        && library_codecs_write(build, scaffold, output);
    free(types);
    return written && fflush(output) == 0;
}
