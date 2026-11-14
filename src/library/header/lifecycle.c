/** @file
 * @brief Emit explicit host lifecycle and diagnostic access with checked ABI wrappers.
 * [EMBED-02](../../../spec/implementation/c-library.md#embed-02) separates
 * context creation, instance construction, entry and quiescent disposal.
 */
#include "internal.h"
/** Emit a public lifecycle wrapper around the matching namespaced implementation. */
static void operation(LibraryHeader *h, const char *name, const char *arguments,
    const char *invocation, const char *description) {
    library_header_print(h, "/** %s */\n%s_ApiResult %s_%s_implementation(%s);\nstatic inline %s_ApiResult %s_%s(%s) {\n",
        description, h->prefix, h->prefix, name, arguments, h->prefix, h->prefix, name, arguments);
    library_header_guard(h);
    library_header_print(h, "    return %s_%s_implementation(%s);\n}\n", h->prefix, name, invocation);
}
void library_header_lifecycle(LibraryHeader *h) {
    const char *p = h->prefix;
    char arguments[512];
    int length = snprintf(arguments, sizeof(arguments), "%s_Context *empty_context", p);
    if (length < 0 || (size_t)length >= sizeof(arguments)) { h->good = false; return; }
    operation(h, "context_create", arguments, "empty_context",
        "Construct context constants/services only; output is initialized all-zero. No executable root/body runs. Factory failure returns its own diagnostic.");
    (void)snprintf(arguments, sizeof(arguments), "%s_Context *context", p);
    operation(h, "context_dispose", arguments, "context",
        "Dispose a quiescent context, invalidate its instances and empty this identity. Never resume a saved continuation; reject active access.");
    (void)snprintf(arguments, sizeof(arguments), "%s_Context context, %s_Instance *instance", p, p);
    operation(h, "instance_dispose", arguments, "context, instance",
        "Destroy one explicit available root and empty its identity. Copies become stale; cleanup failure fails the context while remaining cleanup continues.");
    (void)snprintf(arguments, sizeof(arguments), "%s_Context context, %s_Diagnostic *empty_diagnostic", p, p);
    operation(h, "diagnostic", arguments, "context, empty_diagnostic",
        "Read an independent owned/immutable diagnostic from an available healthy or failed context. Destination is initialized empty.");
    static const char *const names[] = {"result_release", "diagnostic_release"};
    static const char *const types[] = {"ApiResult", "Diagnostic"};
    for (size_t index = 0; index < 2; index++) {
        library_header_print(h,
            "/** Release this sole diagnostic owner and leave it empty. Empty release is a no-op; shallow copies are not owners. */\n"
            "uint32_t %s_%s_implementation(%s_%s *value);\n"
            "static inline uint32_t %s_%s(%s_%s *value) {\n"
            "    uint32_t abi = %s_abi_check();\n"
            "    return abi != UINT32_C(0) ? abi : %s_%s_implementation(value);\n}\n",
            p, names[index], p, types[index], p, names[index], p, types[index], p, p, names[index]);
    }
}
