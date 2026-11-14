/** @file
 * @brief Preserve immutable technical causes independently of context destruction.
 * [EMBED-05](../../../spec/implementation/c-library.md#embed-05) permits
 * documented component-pinned messages; release empties the owner's public value
 * without allocating, retaining a context, or inspecting any freed pool storage.
 */
#include "internal.h"
void library_lifecycle_diagnostics(LibraryHeader *h) {
    const char *p = h->prefix;
    library_header_print(h,
        "/* Diagnostics borrow immutable component code, never context storage. The host\n"
        " * must keep this component loaded until every diagnostic has been released. */\n"
        "static %s_Diagnostic %s_cause(AmaStatus status) {\n"
        "    static const uint8_t fallback[] = ",p,p);
    library_header_literal(h,h->build->component);
    library_header_print(h,
        ";\n    status = ama_status_checked(status);\n"
        "    if (status.code == AMA_OK) { return (%s_Diagnostic){0}; }\n"
        "    return (%s_Diagnostic){.code=status.code,\n"
        "        .message=status.message == NULL ? fallback : (const uint8_t *)status.message,\n"
        "        .length=status.message == NULL ? sizeof(fallback)-1 : status.message_length};\n}\n"
        "static %s_ApiResult %s_result(AmaLibraryResult result) {\n"
        "    return (%s_ApiResult){.code=(uint32_t)result.code,.diagnostic=%s_cause(result.status)};\n}\n"
        "uint32_t %s_diagnostic_release_implementation(%s_Diagnostic *value) {\n"
        "    if (value == NULL) { return UINT32_C(1); }\n"
        "    *value = (%s_Diagnostic){0}; return UINT32_C(0);\n}\n"
        "uint32_t %s_result_release_implementation(%s_ApiResult *value) {\n"
        "    if (value == NULL) { return UINT32_C(1); }\n"
        "    *value = (%s_ApiResult){0}; return UINT32_C(0);\n}\n",
        p,p,p,p,p,p,p,p,p,p,p,p);
}
