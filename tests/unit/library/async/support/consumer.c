/** @file
 * @brief Derive an SDK-free dynamic consumer from exact producer identities and its published external-symbol inventory.
 */
/** Use only explicit producer identities; no basename/hash reconstruction supplies an ABI name. */
static const char *public_schema(const LibraryBuild *build, const char *name) {
    for (const LibrarySchema *schema = build->schemas; schema != NULL; schema = schema->next) {
        if (strcmp(schema->name, name) == 0) { return schema->identity->internal_name; }
    }
    assert(false);
    return NULL;
}
/** Find the producer's explicitly selected export. */
static const char *public_export(const LibraryBuild *build, const char *name) {
    for (const LibraryExport *exported = build->exports; exported != NULL; exported = exported->next) {
        if (strcmp(exported->name, name) == 0) { return exported->identity->internal_name; }
    }
    assert(false);
    return NULL;
}
/** Load only actual published symbols, using the matching header's checked function pointer types. */
static void loader_inventory(FILE *output, unsigned phase) {
    FILE *input = fopen("generated/component/symbols.txt", "rb");
    assert(input != NULL);
    char symbol[256];
    while (fgets(symbol, sizeof(symbol), input) != NULL) {
        size_t length = strlen(symbol);
        assert(length > 1 && symbol[length - 1] == '\n');
        symbol[length - 1] = '\0';
        int written;
        if (phase == 0) { written = fprintf(output, "#define %s (*%s_slot)\n", symbol, symbol); }
        else if (phase == 1) {
            written = fprintf(output, "{ void *address=dlsym(image,\"%s\"); assert(address!=NULL);"
                " _Static_assert(sizeof(%s_slot)==sizeof(address),\"supported POSIX function pointer size\");"
                " memcpy(&%s_slot,&address,sizeof(address)); }\n", symbol, symbol, symbol);
        } else { written = fprintf(output, "%s_slot=NULL;\n", symbol); }
        assert(written > 0);
    }
    assert(!ferror(input));
    int closed = fclose(input);
    assert(closed == 0);
}
/** Compile a host with public headers and backend controls only; the private SDK is deliberately absent from its include arguments. */
static void build_consumer(const LibraryBuild *build, const char *backend, const char *include) {
    FILE *output = fopen("generated/client.c", "wb");
    assert(output != NULL);
    int written = fputs("#include <dlfcn.h>\n#include <assert.h>\n#include <stdio.h>\n#include <string.h>\n", output);
    assert(written != EOF);
    loader_inventory(output, 0);
    written = fprintf(output, "#include \"component/public.h\"\n#define API(name) %s_ ## name\n#define RUN(name) %s_ ## name\n"
        "#define INVALID(name) %s_ ## name\n#define TEXT(name) %s_ ## name\n#define FLAG(name) %s_ ## name\n",
        build->identity->internal_name, public_export(build, "run"), public_export(build, "invalid"),
        public_schema(build, "text"), public_schema(build, "flag"));
    assert(written > 0);
    written = fputs("static void *image;\nstatic void load_image(void) {\n"
        "image=dlopen(\"generated/component/library.so\",RTLD_NOW|RTLD_LOCAL);"
        " if(image==NULL) fprintf(stderr,\"dlopen: %s\\n\",dlerror()); assert(image!=NULL);\n", output);
    assert(written != EOF);
    loader_inventory(output, 1);
    written = fputs("assert(API(code_can_unload)()==0); }\nstatic void unload_image(void) {\n"
        "assert(API(code_can_unload)()==0); int closed=dlclose(image); assert(closed==0);\n", output);
    assert(written != EOF);
    loader_inventory(output, 2);
    written = fputs("image=NULL; }\n#include \"" TEST_ASYNC_HOST "/consumer/host.c\"\n", output);
    assert(written != EOF);
    int closed = fclose(output);
    assert(closed == 0);
    char *arguments[32] = {TEST_C_COMPILER, "-std=c11", "-D_POSIX_C_SOURCE=200809L", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-O1", "-g", "-UNDEBUG", "-Igenerated", "-I", (char *)include};
    size_t count = 13;
#ifdef TEST_THREAD_SANITIZER
    arguments[count++] = "-fsanitize=thread";
    arguments[count++] = "-fno-omit-frame-pointer";
    arguments[count++] = "-fno-pie";
    arguments[count++] = "-no-pie";
#elif defined(TEST_SANITIZERS)
    arguments[count++] = "-fsanitize=address,undefined";
    arguments[count++] = "-fno-omit-frame-pointer";
    arguments[count++] = "-fno-pie";
    arguments[count++] = "-no-pie";
#endif
    arguments[count++] = "generated/client.c";
    arguments[count++] = (char *)backend;
    arguments[count++] = "-ldl";
    arguments[count++] = "-pthread";
    arguments[count++] = "-o";
    arguments[count++] = "generated/consumer";
    arguments[count] = NULL;
    bool compiled = library_package_process(arguments, stderr);
    assert(compiled);
    char *execute[] = {"generated/consumer", NULL};
    bool executed = library_package_process(execute, stderr);
    assert(executed);
}
