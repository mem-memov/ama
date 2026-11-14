/** @file
 * @brief Own counted configuration sources and test-only project files for library validation.
 */
#ifndef AMA_LIBRARY_TEST_COMMON
#define AMA_LIBRARY_TEST_COMMON
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../source/common.c"
#include "library/model.h"
/** A complete no-interface export configuration, independent of actual module discovery. */
static const char library_prefix[] = "ama-library | 1\ncomponent | example.component\nabi | ama-c-1\ntarget | x86_64-linux-gnu\ncalling-convention | c\n";
/** Create a test-owned directory; retain failures instead of accepting a bad fixture. */
static inline void directory(const char *path) {
    int result = mkdir(path, 0700);
    assert(result == 0 || errno == EEXIST);
}
/** Write complete immutable fixture bytes before any compiler owner borrows them. */
static inline void file(const char *path, const char *text) {
    FILE *stream = fopen(path, "wb");
    assert(stream != NULL);
    size_t written = fwrite(text, 1, strlen(text), stream);
    int closed = fclose(stream);
    assert(written == strlen(text) && closed == 0);
}
/** Add the exact prefix and selected records to the stable source-manager fixture. */
static const SourceText *configuration(const char *tail) {
    char text[8192];
    int length = snprintf(text, sizeof(text), "%s%s", library_prefix, tail);
    assert(length > 0 && (size_t)length < sizeof(text));
    const SourceText *source = NULL;
    SourceStatus status = source_manager_add(&sources, "component.exports", (const unsigned char *)text, (size_t)length, &source);
    assert(status == SOURCE_OK);
    return source;
}
#endif
