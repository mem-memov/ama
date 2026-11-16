/** @file
 * @brief Compare complete framed keys when two headers share a candidate C namespace.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) and
 * [NAME-02](../../../spec/implementation/compiler.md#name-02) forbid hash-only
 * identity merging, including across independently generated consumer headers.
 */
#include <stdint.h>
#include "internal.h"
/** Pack at most eight original key bytes in a specified order; the separate length preserves padding identity. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [NAME-02](../../../spec/implementation/compiler.md#name-02). */
static uint64_t word(const LibraryBuild *build, size_t start) {
    uint64_t value = 0;
    for (size_t i = 0; i < 8; i++) {
        value <<= 8;
        if (i < build->identity->encoding_length - start) { value |= build->identity->encoding[start + i]; }
    }
    return value;
}
/** Emit an exact unsigned constant without relying on the formatting width of uint64_t. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [NAME-02](../../../spec/implementation/compiler.md#name-02). */
static void number(LibraryHeader *h, uint64_t value) {
    static const char digits[] = "0123456789abcdef";
    library_header_print(h, "UINT64_C(0x");
    for (size_t i = 0; i < 16; i++) { library_header_print(h, "%c", digits[(value >> ((15 - i) * 4)) & 15]); }
    library_header_print(h, ")");
}
void library_header_identity_guard(LibraryHeader *h) {
    const char *m = h->macro_prefix;
    size_t length = h->build->identity->encoding_length;
    size_t count = length / 8 + (length % 8 != 0);
    library_header_print(h,
        "#if defined(%s_PUBLIC_H)\n#if %s_KEY_LENGTH != %zu\n"
        "#error Conflicting complete Ama component keys share a candidate namespace\n#else\n", m,m,length);
    library_header_print(h, "_Static_assert(\n");
    for (size_t i = 0; i < count; i++) {
        library_header_print(h, "    %s_KEY_%zu == ", m,i);
        number(h, word(h->build, i * 8));
        library_header_print(h, i + 1 == count ? ",\n" : " &&\n");
    }
    library_header_print(h, "    \"Conflicting complete Ama component keys share a candidate namespace\");\n");
    library_header_print(h, "#endif\n#else\n#define %s_PUBLIC_H\n#define %s_KEY_LENGTH %zu\n", m,m,length);
    for (size_t i = 0; i < count; i++) {
        library_header_print(h, "#define %s_KEY_%zu ", m,i);
        number(h, word(h->build, i * 8));
        library_header_print(h, "\n");
    }
}
