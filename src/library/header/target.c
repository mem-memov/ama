/** @file
 * @brief Select only explicitly supported consumer C-ABI profiles before header emission.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires
 * target/ABI agreement before crossing C boundaries; metadata is not a target guess.
 */
#include <assert.h>
#include <string.h>
#include "library/model.h"
bool library_header_target(const LibraryBuild *build, Diagnostic *error) {
    assert(build->valid);
    static const char *const supported[] = {"x86_64-linux-gnu", "x86_64-pc-linux-gnu"};
    for (size_t index = 0; index < sizeof(supported) / sizeof(*supported); index++) {
        if (strcmp(build->target, supported[index]) == 0) { return true; }
    }
    if (error != NULL) {
        *error = (Diagnostic){.code = DIAGNOSTIC_UNSUPPORTED_FEATURE,
            .message = "Consumer headers currently support the GNU-compatible x86-64 Linux ELF LP64 System V C ABI",
            .subject = build->target, .primary = {.span = build->target_site}};
    }
    return false;
}
