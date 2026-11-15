/** @file
 * @brief Emit detached callback code pins and a loader-barrier quiescence query.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires
 * pins to outlive callback return/retirement, independently of context cleanup.
 * The external loader excludes calls until after the actual system unload.
 */
#include "internal.h"
void library_lifecycle_code(LibraryHeader *h) {
    const char *p=h->prefix;
    library_header_print(h,
        "typedef struct %s_Pin { struct %s_Pin *next; uint64_t identity; } %s_Pin;\n"
        "static %s_Pin *%s_pins;\nstatic uint64_t %s_next_pin=UINT64_C(1);\n"
        "uint32_t %s_code_pin_implementation(%s_CodePin *token) {\n"
        "    if (token == NULL || token->opaque[0] != 0 || token->opaque[1] != 0) { return UINT32_C(1); }\n"
        "    call_once(&%s_once,%s_setup);\n"
        "    if (%s_setup_status.code != AMA_OK) { return UINT32_C(5); }\n"
        "    %s_Pin *pin=malloc(sizeof(*pin));\n"
        "    if (pin == NULL) { return UINT32_C(5); }\n"
        "    ama_library_lock(&%s_service);\n"
        "    if (%s_next_pin == 0 || !ama_library_code_retain(&%s_code)) {\n"
        "        ama_library_unlock(&%s_service); free(pin); return UINT32_C(5);\n    }\n"
        "    *pin=(%s_Pin){.next=%s_pins,.identity=%s_next_pin};\n"
        "    %s_next_pin=%s_next_pin == UINT64_MAX ? 0 : %s_next_pin+1;\n"
        "    %s_pins=pin; token->opaque[0]=(uintptr_t)&%s_code; token->opaque[1]=pin->identity;\n"
        "    ama_library_unlock(&%s_service); return UINT32_C(0);\n}\n",
        p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p);
    library_header_print(h,
        "uint32_t %s_code_unpin_implementation(%s_CodePin *token) {\n"
        "    if (token == NULL) { return UINT32_C(1); }\n"
        "    if (token->opaque[0] == 0 && token->opaque[1] == 0) { return UINT32_C(0); }\n"
        "    if (token->opaque[0] != (uintptr_t)&%s_code || token->opaque[1] == 0) { return UINT32_C(1); }\n"
        "    call_once(&%s_once,%s_setup);\n"
        "    if (%s_setup_status.code != AMA_OK) { return UINT32_C(5); }\n"
        "    ama_library_lock(&%s_service);\n"
        "    %s_Pin **link=&%s_pins;\n"
        "    while (*link != NULL && (*link)->identity != token->opaque[1]) { link=&(*link)->next; }\n"
        "    %s_Pin *pin=*link;\n"
        "    if (pin == NULL) { ama_library_unlock(&%s_service); return UINT32_C(1); }\n"
        "    *link=pin->next; *token=(%s_CodePin){0};\n"
        "    bool released=ama_library_code_release(&%s_code); (void)released; assert(released);\n"
        "    ama_library_unlock(&%s_service); free(pin); return UINT32_C(0);\n}\n"
        "uint32_t %s_code_can_unload_implementation(void) {\n"
        "    call_once(&%s_once,%s_setup);\n"
        "    if (%s_setup_status.code != AMA_OK) { return UINT32_C(5); }\n"
        "    ama_library_lock(&%s_service);\n"
        "    bool ready=%s_service.first == NULL && ama_library_code_quiescent(&%s_code);\n"
        "    ama_library_unlock(&%s_service);\n"
        "    return ready ? UINT32_C(0) : UINT32_C(2);\n}\n",
        p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p,p);
}
