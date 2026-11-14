/** @file
 * @brief Compose self-contained consumer-header generation and safe literal emission.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) keeps native
 * authoring headers out of the consumer interface; output publication remains separate.
 */
#include <assert.h>
#include <stdarg.h>
#include "internal.h"
void library_header_print(LibraryHeader *header, const char *format, ...) {
    if (!header->good) { return; }
    va_list arguments;
    va_start(arguments, format);
    if (vfprintf(header->output, format, arguments) < 0) { header->good = false; }
    va_end(arguments);
}
void library_header_literal(LibraryHeader *header, const char *text) {
    library_header_print(header, "\"");
    for (const unsigned char *p = (const unsigned char *)text; *p != 0; p++) {
        /* Three octal digits preserve each original octet independently of
         * execution charset and prevent quote, trigraph and following-digit effects.
         */
        library_header_print(header, "\\%03o", (unsigned)*p);
    }
    library_header_print(header, "\"");
}
bool library_header_write(const LibraryBuild *build, FILE *output) {
    assert(build->valid);
    if (!library_header_target(build, NULL)) { return false; }
    LibraryHeader header = {.build = build, .output = output, .prefix = build->identity->internal_name, .good = true};
    for (size_t index = 0; index < sizeof(header.macro_prefix); index++) {
        char byte = header.prefix[index];
        header.macro_prefix[index] = byte >= 'a' && byte <= 'z' ? (char)(byte - 'a' + 'A') : byte;
        if (byte == 0) { break; }
    }
    library_header_preamble(&header);
    library_header_values(&header);
    library_header_abi(&header);
    library_header_lifecycle(&header);
    library_header_snapshots(&header);
    library_header_exports(&header);
    library_header_print(&header, "\n#endif /* %s_PUBLIC_H */\n", header.macro_prefix);
    return header.good && fflush(output) == 0;
}
const LibraryBinding *library_header_binding(const LibraryBuild *build,
    const LibraryExport *exported, const SemanticSymbol *field) {
    for (const LibraryBinding *binding = build->bindings; binding != NULL; binding = binding->next) {
        if (binding->exported == exported && binding->field == field) { return binding; }
    }
    assert(false);
    return NULL;
}
void library_header_guard(LibraryHeader *header) {
    library_header_print(header,
        "    uint32_t abi = %s_abi_check();\n"
        "    if (abi != UINT32_C(0)) { return (%s_ApiResult){.code = abi}; }\n",
        header->prefix, header->prefix);
}
