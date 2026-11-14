/** @file
 * @brief Gate public structure calls through an exact scalar C-ABI bootstrap.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) rejects stale
 * headers and layout-affecting client options before passing public C structures.
 */
#include <assert.h>
#include "internal.h"
/** Add one complete object's size/alignment and its named field offset. */
static void field(LibraryHeader *h, const char *type, const char *member) {
    library_header_print(h, "        sizeof(%s), _Alignof(%s), offsetof(%s, %s),\n", type, type, type, member);
}
/** Spell a public type in bounded scratch; generated prefixes have a fixed maximum length. */
static void member(LibraryHeader *h, const char *prefix, const char *suffix, const char *name) {
    char type[128];
    int length = snprintf(type, sizeof(type), "%s_%s", prefix, suffix);
    if (length < 0 || (size_t)length >= sizeof(type)) { h->good = false; return; }
    field(h, type, name);
}
/** Emit every public layout fact in the same order for consumer and compiled receiver. */
static void layout(LibraryHeader *h, const char *name) {
    const char *p = h->prefix;
    library_header_print(h, "    static const size_t %s[] = {\n        sizeof(size_t), _Alignof(size_t), sizeof(uintptr_t), _Alignof(uintptr_t),\n", name);
    member(h, p, "Context", "opaque"); member(h, p, "Instance", "opaque");
    member(h, p, "Bytes", "data"); member(h, p, "Bytes", "length");
    member(h, p, "Diagnostic", "code"); member(h, p, "Diagnostic", "message");
    member(h, p, "Diagnostic", "length"); member(h, p, "Diagnostic", "opaque");
    member(h, p, "ApiResult", "code"); member(h, p, "ApiResult", "diagnostic");
    for (const LibrarySchema *s = h->build->schemas; s != NULL; s = s->next) {
        const char *n = s->identity->internal_name;
        if (s->kind == LIBRARY_INTEGER) { member(h, n, "Borrowed", "negative"); member(h, n, "Borrowed", "magnitude"); }
        else if (s->kind == LIBRARY_CONDITION) { member(h, n, "Borrowed", "value"); }
        else if (s->kind == LIBRARY_FAILURE) { member(h, n, "Borrowed", "failed"); member(h, n, "Borrowed", "description"); }
        else { member(h, n, "Borrowed", s->kind == LIBRARY_TEXT ? "data" : "items"); member(h, n, "Borrowed", "length"); }
    }
    library_header_print(h, "    };\n");
}
/** Emit the full framed component key; digest equality is never identity equality. */
static void identity(LibraryHeader *h, const char *name) {
    library_header_print(h, "    static const uint8_t %s[%zu] = {", name, h->build->identity->encoding_length);
    for (size_t index = 0; index < h->build->identity->encoding_length; index++) {
        library_header_print(h, "%s%u", index == 0 ? "" : ",", (unsigned)h->build->identity->encoding[index]);
    }
    library_header_print(h, "};\n");
}
void library_header_abi(LibraryHeader *h) {
    const char *p = h->prefix;
    library_header_print(h,
        "/** Bootstrap uses only scalar integers and valid bounded C borrows.\n"
        " * Public calls below invoke it automatically; direct *_implementation calls bypass the contract. */\n"
        "uint32_t %s_abi_implementation(uint32_t edition, const uint8_t *identity, size_t identity_length, const size_t *layout, size_t layout_count);\n"
        "/** Validate the component key and every public size/alignment/offset before ABI use. */\n"
        "static inline uint32_t %s_abi_check(void) {\n", p, p);
    identity(h, "identity");
    layout(h, "layout");
    library_header_print(h, "    return %s_abi_implementation(UINT32_C(1), identity, sizeof(identity), layout, sizeof(layout) / sizeof(*layout));\n}\n", p);
}
bool library_abi_write(const LibraryBuild *build, FILE *output) {
    assert(build->valid);
    if (!library_header_target(build, NULL)) { return false; }
    LibraryHeader h = {.build = build, .output = output, .prefix = build->identity->internal_name, .good = true};
    library_header_print(&h,
        "/** Check version, complete ABI key and all layout facts without touching any Ama context.\n"
        " * Caller buffers are valid bounded C borrows; mismatch returns before structure-based calls. */\n"
        "uint32_t %s_abi_implementation(uint32_t edition, const uint8_t *identity, size_t identity_length, const size_t *layout, size_t layout_count) {\n", h.prefix);
    identity(&h, "expected_identity");
    layout(&h, "expected_layout");
    library_header_print(&h,
        "    if (edition != UINT32_C(1) || identity_length != sizeof(expected_identity)\n"
        "        || layout_count != sizeof(expected_layout) / sizeof(*expected_layout)\n"
        "        || identity == NULL || layout == NULL) { return UINT32_C(4); }\n"
        "    for (size_t i = 0; i < sizeof(expected_identity); i++) { if (identity[i] != expected_identity[i]) { return UINT32_C(4); } }\n"
        "    for (size_t i = 0; i < layout_count; i++) { if (layout[i] != expected_layout[i]) { return UINT32_C(4); } }\n"
        "    return UINT32_C(0);\n}\n");
    return h.good && fflush(output) == 0;
}
