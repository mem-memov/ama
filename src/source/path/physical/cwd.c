/** @file
 * @brief Obtain actual cwd bytes without environment aliases or fixed path limits.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) preserves exact paths;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns every failed capacity attempt.
 */
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include "internal.h"
SourcePhysicalStatus source_physical_cwd(SourceArena *arena, const char **output, SourcePhysicalIssue *issue) {
    size_t capacity = 128;
    for (;;) {
        char *buffer = source_arena_allocate(arena, capacity);
        if (!buffer) { return source_physical_fail(issue, SOURCE_PHYSICAL_STORAGE, NULL, PATH_IO_ERROR); }
        if (getcwd(buffer, capacity)) { *output = buffer; return SOURCE_PHYSICAL_OK; }
        if (errno != ERANGE) { return source_physical_fail(issue, SOURCE_PHYSICAL_IO_ERROR, NULL, PATH_IO_ERROR); }
        if (capacity > SIZE_MAX / 2) { return source_physical_fail(issue, SOURCE_PHYSICAL_STORAGE, NULL, PATH_IO_ERROR); }
        capacity *= 2;
    }
}
