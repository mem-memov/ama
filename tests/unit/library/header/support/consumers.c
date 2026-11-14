/** @file
 * @brief Compile independent translation units against two strongly namespaced public headers.
 */
/** Public snapshots keep failure condition/message together and require explicit ownership operations. */
static void it_compiles_independent_consumers_and_two_component_headers(void) {
    prepare_project();
    setup_test();
    LibraryBuild first, second;
    component(&first, "example.first");
    component(&second, "example.second");
    header(&first, "generated/first.h");
    header(&second, "generated/second.h");
    const char *p = first.identity->internal_name;
    const char *e = first.exports->identity->internal_name;
    const char *f = first.schemas->next->next->next->identity->internal_name;
    const char *t = first.schemas->identity->internal_name;
    char source[16384];
    int length = snprintf(source, sizeof(source),
        "/** @file Pure consumer using one coherent failure snapshot and explicit ownership. */\n"
        "#include \"first.h\"\n#include \"first.h\"\n#include \"second.h\"\n"
        "uint32_t consume(%s_Context context, %s_Instance instance) {\n"
        "    %s_Borrowed input = {(const uint8_t *)\"hello\",5};\n"
        "    %s_Value *failure = NULL, *copy = NULL; uint32_t outcome = 0;\n"
        "    %s_ApiResult result = %s_entry(context,instance,&input,&failure,&outcome);\n"
        "    uint32_t released = %s_result_release(&result);\n"
        "    result = %s_clone(failure,&copy);\n"
        "    released |= %s_result_release(&result);\n"
        "    %s_Borrowed view = {0}; result = %s_view(copy,&view);\n"
        "    uint32_t failed = view.failed; size_t description_length = view.description.length;\n"
        "    released |= %s_result_release(&result);\n"
        "    result = %s_release(&copy); released |= %s_result_release(&result);\n"
        "    result = %s_release(&failure); released |= %s_result_release(&result);\n"
        "    return released + failed + (uint32_t)description_length;\n}\n",
        p,p,t,f,p,e,p,f,p,f,f,p,f,p,f,p);
    assert(length > 0 && (size_t)length < sizeof(source));
    file("generated/first-consumer.c", source);
    char diagnostic[32768];
    bool compiled = compile("generated/first-consumer.c", NULL, "generated/first-consumer.o", false, NULL, diagnostic, sizeof(diagnostic));
    if (!compiled) { fprintf(stderr, "%s", diagnostic); }
    assert(compiled && diagnostic[0] == 0);
    const char *q = second.identity->internal_name;
    length = snprintf(source, sizeof(source),
        "/** @file Another independent translation unit includes the headers in reverse order. */\n"
        "#include \"second.h\"\n#include \"first.h\"\n"
        "uint32_t other(void) { %s_Context a={0}; %s_Context b={0}; (void)a;(void)b;return %s_abi_check(); }\n", p,q,q);
    assert(length > 0 && (size_t)length < sizeof(source));
    file("generated/second-consumer.c", source);
    compiled = compile("generated/second-consumer.c", NULL, "generated/second-consumer.o", false, NULL, diagnostic, sizeof(diagnostic));
    if (!compiled) { fprintf(stderr, "%s", diagnostic); }
    assert(compiled && diagnostic[0] == 0);
    library_build_destroy(&second);
    library_build_destroy(&first);
    destroy_test();
}
