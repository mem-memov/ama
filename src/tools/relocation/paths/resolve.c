/** @file
 * @brief Resolve physical request paths without accepting symlinks or spelling aliases.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) requires exact
 * directory entries; [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * permits only a missing final destination beneath an existing parent.
 */
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../internal.h"
#include "source/path/exact.h"
#include "source/unicode/unicode.h"

/** ERANGE means an insufficient exact buffer, not a guessed maximum pathname length. */
static const char *working_directory(RelocationPreview *preview) {
    size_t capacity = 128;
    for (;;) {
        char *buffer = source_arena_allocate(&preview->arena,capacity);
        if (buffer == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,NULL,NULL); return NULL; }
        if (getcwd(buffer,capacity) != NULL) { return buffer; }
        if (errno != ERANGE) { relocation_fail(preview,RELOCATION_IO_ERROR,NULL,NULL); return NULL; }
        if (capacity > SIZE_MAX/2) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,NULL,NULL); return NULL; }
        capacity *= 2;
    }
}

/** The current directory is already verified; its lexical parent therefore follows actual traversal. */
static const char *parent_directory(RelocationPreview *preview, const char *current) {
    char *parent = source_arena_copy(&preview->arena,current);
    if (parent == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,current,NULL); return NULL; }
    char *separator = strrchr(parent,'/');
    if (separator == parent) { separator[1] = 0; }
    else { *separator = 0; }
    return parent;
}

/** Verify every traversed ordinary component before interpreting subsequent dot-dot components. */
static const char *resolve_path(RelocationPreview *preview, const char *input, bool missing_final, bool final_view) {
    if (input == NULL || *input == 0) { relocation_fail(preview,RELOCATION_INVALID_PATH,NULL,NULL); return NULL; }
    char *copy = source_arena_copy(&preview->arena,input);
    if (copy == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,NULL,NULL); return NULL; }
    size_t error;
    if (!utf8_validate((const unsigned char *)copy,strlen(copy),&error) || strchr(copy,'\r') != NULL) {
        relocation_fail(preview,RELOCATION_UNSUPPORTED_PATH,copy,NULL); return NULL;
    }
    const char *current = *copy == '/' ? "/" : working_directory(preview);
    if (current == NULL) { return NULL; }
    char *cursor = copy;
    while (*cursor != 0) {
        while (*cursor == '/') { cursor++; }
        if (*cursor == 0) { break; }
        char *component = cursor;
        while (*cursor != 0 && *cursor != '/') { cursor++; }
        if (*cursor != 0) { *cursor++ = 0; }
        char *remaining = cursor;
        while (*remaining == '/') { remaining++; }
        if (strcmp(component,".") == 0) { continue; }
        if (strcmp(component,"..") == 0) {
            current = parent_directory(preview,current);
            if (current == NULL) { return NULL; }
            continue;
        }
        const char *full = source_arena_join(&preview->arena,current,strcmp(current,"/") == 0 ? "" : "/",component);
        if (full == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,current,NULL); return NULL; }
        struct stat metadata;
        const char *physical = full;
        if (final_view && relocation_contains(preview->destination,full)) {
            physical = source_arena_join(&preview->arena,preview->source,"",full+strlen(preview->destination));
            if (physical == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,full,NULL); return NULL; }
        } else if (final_view && relocation_contains(preview->source,full)) {
            relocation_fail(preview,RELOCATION_INVALID_PATH,full,NULL); return NULL;
        }
        char *parent = source_arena_copy(&preview->arena,physical);
        if (parent == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,full,NULL); return NULL; }
        char *slash = strrchr(parent,'/');
        const char *name = physical+(slash-parent)+1;
        if (slash == parent) { slash[1] = 0; } else { *slash = 0; }
        PathProbe probe = path_exact_child(parent,name,physical,&metadata);
        if (probe == PATH_FOUND && missing_final && *remaining == 0) { return full; }
        if (probe == PATH_MISSING && missing_final && *remaining == 0) { return full; }
        if (probe != PATH_FOUND || !S_ISDIR(metadata.st_mode)) {
            RelocationStatus status = probe == PATH_UNSUPPORTED ? RELOCATION_UNSUPPORTED_PATH
                : probe == PATH_FOUND || probe == PATH_MISSING ? RELOCATION_INVALID_PATH : RELOCATION_IO_ERROR;
            relocation_fail(preview,status,full,NULL); return NULL;
        }
        current = full;
    }
    return current;
}

/** Resolve actual endpoints and original roots using exact current filesystem observations. */
const char *relocation_path(RelocationPreview *preview, const char *input, bool missing_final) {
    return resolve_path(preview,input,missing_final,false);
}
/** Resolve roots below D through their original counterpart below S, without temporary filesystem writes. */
const char *relocation_final_path(RelocationPreview *preview, const char *input) {
    return resolve_path(preview,input,false,true);
}
