/** @file
 * @brief Emit typed snapshot viewing, explicit cloning and consuming release.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) gives snapshots
 * independent lifetimes and forbids shallow C assignment as duplication.
 */
#include "internal.h"
void library_header_snapshots(LibraryHeader *h) {
    for (const LibrarySchema *s = h->build->schemas; s != NULL; s = s->next) {
        const char *n = s->identity->internal_name, *p = h->prefix;
        library_header_print(h,
            "/** Borrow snapshot data until its owner is released; no Ama/context storage is exposed.\n"
            " * The snapshot stays usable after source instance/context disposal. Failed view preserves output. */\n"
            "%s_ApiResult %s_view_implementation(const %s_Value *value, %s_Borrowed *view);\n"
            "static inline %s_ApiResult %s_view(const %s_Value *value, %s_Borrowed *view) {\n",
            p, n, n, n, p, n, n, n);
        library_header_guard(h);
        library_header_print(h, "    return %s_view_implementation(value, view);\n}\n", n);
        library_header_print(h,
            "/** Create an independent releasing owner; *empty_destination is initialized NULL.\n"
            " * Failure preserves source and destination; no context or continuation is copied.\n"
            " * The returned API result also transfers any diagnostic owner. */\n");
        /* The manifest reserves the public clone/release names. Implementations
         * use a distinct suffix so these public names remain ABI-checked inline wrappers.
         */
        library_header_print(h,
            "%s_ApiResult %s_clone_implementation(const %s_Value *source, %s_Value **empty_destination);\n"
            "static inline %s_ApiResult %s_clone(const %s_Value *source, %s_Value **empty_destination) {\n",
            p, n, n, n, p, n, n, n);
        library_header_guard(h);
        library_header_print(h, "    return %s_clone_implementation(source, empty_destination);\n}\n", n);
        library_header_print(h,
            "/** Consume the sole owning pointer and leave it NULL. Releasing initialized NULL is a no-op.\n"
            " * Do not release uninitialized storage or a shallow pointer copy as another owner. */\n"
            "%s_ApiResult %s_release_implementation(%s_Value **value);\n"
            "static inline %s_ApiResult %s_release(%s_Value **value) {\n", p, n, n, p, n, n);
        library_header_guard(h);
        library_header_print(h, "    return %s_release_implementation(value);\n}\n", n);
    }
}
