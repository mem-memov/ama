/** @file
 * @brief Emit checked lifecycle thunks without exposing private runtime representations.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) keeps numeric
 * controls distinct from dereferenceable addresses; ABI checks run in the public
 * wrappers before any structure-based thunk is called.
 */
#include "internal.h"
void library_lifecycle_operations(LibraryHeader *h) {
    const char *p = h->prefix;
    library_header_print(h,
        "static AmaLibraryContext %s_context(%s_Context value) {\n"
        "    return (AmaLibraryContext){(uintptr_t)value.opaque[0],value.opaque[1]};\n}\n"
        "static AmaLibraryInstance %s_instance(%s_Instance value) {\n"
        "    return (AmaLibraryInstance){(uintptr_t)value.opaque[0],value.opaque[1],value.opaque[2]};\n}\n"
        "%s_ApiResult %s_context_create_implementation(%s_Context *output) {\n"
        "    call_once(&%s_once,%s_setup);\n"
        "    if (%s_setup_status.code != AMA_OK) { return %s_result((AmaLibraryResult){AMA_LIBRARY_TECHNICAL_FAILURE,%s_setup_status}); }\n"
        "    AmaLibraryContext context = output == NULL ? (AmaLibraryContext){0} : %s_context(*output);\n"
        "    if (output != NULL && context.owner == 0 && context.identity == 0) {\n"
        "        AmaStatus domain=%s_domain();\n"
        "        if (domain.code != AMA_OK) { return %s_result((AmaLibraryResult){AMA_LIBRARY_TECHNICAL_FAILURE,domain}); }\n    }\n"
        "    AmaLibraryResult result = ama_library_context_create(&%s_service,output == NULL ? NULL : &context);\n"
        "    if (output != NULL) { *output = (%s_Context){{context.owner,context.identity}}; }\n"
        "    return %s_result(result);\n}\n",
        p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p);
    library_header_print(h,
        "%s_ApiResult %s_context_dispose_implementation(%s_Context *output) {\n"
        "    call_once(&%s_once,%s_setup);\n"
        "    AmaLibraryContext context = output == NULL ? (AmaLibraryContext){0} : %s_context(*output);\n"
        "    AmaLibraryResult result = ama_library_context_dispose(&%s_service,output == NULL ? NULL : &context);\n"
        "    if (output != NULL) { *output = (%s_Context){{context.owner,context.identity}}; }\n"
        "    return %s_result(result);\n}\n"
        "%s_ApiResult %s_instance_dispose_implementation(%s_Context context, %s_Instance *output) {\n"
        "    call_once(&%s_once,%s_setup);\n"
        "    AmaLibraryInstance instance = output == NULL ? (AmaLibraryInstance){0} : %s_instance(*output);\n"
        "    AmaLibraryResult result = ama_library_instance_dispose(&%s_service,%s_context(context),output == NULL ? NULL : &instance);\n"
        "    if (output != NULL) { *output = (%s_Instance){{instance.owner,instance.context,instance.identity}}; }\n"
        "    return %s_result(result);\n}\n",
        p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p);
    library_header_print(h,
        "%s_ApiResult %s_diagnostic_implementation(%s_Context context, %s_Diagnostic *output) {\n"
        "    call_once(&%s_once,%s_setup);\n"
        "    if (output == NULL || output->code != 0 || output->message != NULL || output->length != 0\n"
        "        || output->opaque[0] != 0 || output->opaque[1] != 0 || output->opaque[2] != 0 || output->opaque[3] != 0) {\n"
        "        return %s_result((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)});\n    }\n"
        "    AmaStatus status = ama_status_success();\n"
        "    AmaLibraryResult result = ama_library_diagnostic(&%s_service,%s_context(context),&status);\n"
        "    if (result.code == AMA_LIBRARY_OK) { *output = %s_cause(status); }\n"
        "    return %s_result(result);\n}\n",
        p,p,p,p,p,p,p,p,p,p,p);
}
