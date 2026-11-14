/** @file
 * @brief Describe readonly typed borrows and opaque independently owned snapshots.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) requires
 * declared representations without live Ama payload or reference addresses.
 */
#include "internal.h"
void library_header_values(LibraryHeader *h) {
    for (const LibrarySchema *s = h->build->schemas; s != NULL; s = s->next) {
        const char *n = s->identity->internal_name;
        library_header_print(h, "/** Public schema %s transports nominal %s%s. */\n", s->name,
            s->type->module->source->logical_path, s->kind == LIBRARY_LIST ? " owning-list elements" : " values");
        library_header_print(h,
            "/** Readonly input or snapshot view; its owner outlives the complete borrow. */\n"
            "typedef struct %s_Borrowed %s_Borrowed;\n"
            "/** Opaque owned immutable snapshot. Initialize its pointer NULL; use explicit clone. */\n"
            "typedef struct %s_Value %s_Value;\n", n, n, n, n);
    }
    for (const LibrarySchema *s = h->build->schemas; s != NULL; s = s->next) {
        const char *n = s->identity->internal_name;
        if (s->kind == LIBRARY_INTEGER) {
            library_header_print(h,
                "/** Canonical sign/magnitude: negative is 0/1; big-endian magnitude has no leading zero.\n"
                " * Zero is one zero octet and negative=0; bytes are borrowed only for this operation. */\n"
                "struct %s_Borrowed { uint8_t negative; %s_Bytes magnitude; };\n", n, h->prefix);
        } else if (s->kind == LIBRARY_TEXT) {
            library_header_print(h,
                "/** Exact UTF-8 octets, including embedded zero; data may be NULL only at length zero. */\n"
                "struct %s_Borrowed { const uint8_t *data; size_t length; };\n", n);
        } else if (s->kind == LIBRARY_CONDITION) {
            library_header_print(h, "/** Explicit Boolean transport; values other than 0/1 are invalid. */\nstruct %s_Borrowed { uint8_t value; };\n", n);
        } else if (s->kind == LIBRARY_FAILURE) {
            library_header_print(h,
                "/** One failure value: failed is 0/1; false requires an empty UTF-8 description.\n"
                " * These members encode one Ama result, not an extra success/reason pair. */\n"
                "struct %s_Borrowed { uint8_t failed; %s_Bytes description; };\n", n, h->prefix);
        } else {
            library_header_print(h,
                "/** Ordered array of declared scalar values; items is NULL only at length zero. */\n"
                "struct %s_Borrowed { const %s_Borrowed *items; size_t length; };\n", n, s->element->identity->internal_name);
        }
    }
}
