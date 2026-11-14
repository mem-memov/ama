/** @file
 * @brief Generate component-private lifecycle service setup from explicit resolved type IDs.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) requires
 * selected nominal exports; runtime ordinals are supplied by the project layout,
 * never inferred from filenames or public hash spellings.
 */
#include <assert.h>
#include "internal.h"
bool library_lifecycle_write(const LibraryBuild *build, const uint64_t *types,
    size_t count, FILE *output) {
    assert(build->valid);
    if (!library_header_target(build,NULL) || types == NULL || count != build->export_count) { return false; }
    for (size_t i = 0; i < count; i++) { if (types[i] == 0) { return false; } }
    LibraryHeader h = {.build=build,.output=output,.prefix=build->identity->internal_name,.good=true};
    library_header_print(&h,
        "#include <threads.h>\n#include <stdlib.h>\n#include \"runtime/library/lifecycle.h\"\n"
        "static AmaLibraryService %s_service;\nstatic once_flag %s_once = ONCE_FLAG_INIT;\n"
        "static AmaStatus %s_setup_status;\nstatic AmaContextDefinition %s_definition;\n"
        "static const AmaLibraryExport %s_exports[] = {",h.prefix,h.prefix,h.prefix,h.prefix,h.prefix);
    for (size_t i = 0; i < count; i++) {
        library_header_print(&h,"%s{UINT64_C(%zu),UINT64_C(%llu)}",i == 0 ? "" : ",",i+1,(unsigned long long)types[i]);
    }
    library_header_print(&h,
        "};\nstatic void *%s_allocate(void *unused, size_t size) { (void)unused; return malloc(size); }\n"
        "static void %s_free(void *unused, void *value) { (void)unused; free(value); }\n"
        "static void %s_setup(void) {\n    %s_definition = ama_project_definition;\n    %s_definition.component = ",
        h.prefix,h.prefix,h.prefix,h.prefix,h.prefix);
    library_header_literal(&h,build->component);
    library_header_print(&h,
        ";\n    %s_setup_status = ama_library_service_init(&%s_service,\n"
        "        (SourceAllocator){NULL,%s_allocate,%s_free}, &%s_definition,%s_exports,%zu);\n}\n",
        h.prefix,h.prefix,h.prefix,h.prefix,h.prefix,h.prefix,count);
    library_lifecycle_diagnostics(&h);
    library_lifecycle_operations(&h);
    size_t i = 1;
    for (const LibraryExport *e = build->exports; e != NULL; e = e->next,i++) {
        library_header_print(&h,
            "%s_ApiResult %s_create_implementation(%s_Context context, %s_Instance *output) {\n"
            "    call_once(&%s_once,%s_setup);\n"
            "    if (%s_setup_status.code != AMA_OK) { return %s_result((AmaLibraryResult){AMA_LIBRARY_TECHNICAL_FAILURE,%s_setup_status}); }\n"
            "    AmaLibraryInstance instance = output == NULL ? (AmaLibraryInstance){0} : %s_instance(*output);\n"
            "    AmaLibraryResult result = ama_library_instance_create(&%s_service,%s_context(context),UINT64_C(%zu),output == NULL ? NULL : &instance);\n"
            "    if (output != NULL) { *output = (%s_Instance){{instance.owner,instance.context,instance.identity}}; }\n"
            "    return %s_result(result);\n}\n",
            h.prefix,e->identity->internal_name,h.prefix,h.prefix,h.prefix,h.prefix,h.prefix,h.prefix,h.prefix,
            h.prefix,h.prefix,h.prefix,i,h.prefix,h.prefix);
    }
    return h.good && fflush(output) == 0;
}
