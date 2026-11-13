/** @file
 * @brief Compile the actual project translation unit and every isolated native interface under both supported optimization profiles.
 * The probes consume production output with independent C type checks; they do
 * not read generated files or infer correctness from emitted substrings.
 */
#include "native/toolchain/profile.h"

/** Require the real C compiler to accept a complete source without a preceding forced include. */
static void compile_translation(const char *path, const char *optimization) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("gcc", "gcc", AMA_TOOLCHAIN_REQUIRED_FLAGS, optimization,
            "-I" TEST_SDK, "-Igenerated/output", "-x", "c", "-fsyntax-only", path, (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/** Keep isolated declarations usable before the assembly and compatible afterwards, with no native context macros escaping. */
static void it_compiles_the_self_contained_assembly_and_isolated_interfaces_in_either_include_order(void) {
    prepare_assembly();
    begin_assembly();
    bool built = build_assembly();
    assert(built);
    for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) { emit_executable_native(type); }
    }
    bool published = native_scaffold_publish(&assembly);
    assert(published);
    const char *optimizations[] = {"-O0", "-O2"};
    for (size_t profile = 0; profile < 2; profile++) {
        compile_translation("generated/output/project.c", optimizations[profile]);
        for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
            if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
            char path[256];
            int length = snprintf(path, sizeof(path), "generated/output/%s", type->interface_name);
            assert(length > 0 && (size_t)length < sizeof(path));
            compile_translation(path, optimizations[profile]);
        }
        for (size_t order = 0; order < 2; order++) {
            FILE *output = fopen("generated/agreement.c", "wb");
            assert(output != NULL);
            int written = order == 0 ? fputs("#include \"project.c\"\n", output) : 0;
            assert(written != EOF);
            for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
                if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
                bool included = scaffold_include(output, type->interface_name);
                assert(included);
            }
            written = order == 1 ? fputs("#include \"project.c\"\n", output) : 0;
            assert(written != EOF);
            written = fputs(
                "#if defined(AMA_NATIVE) || defined(AMA_NATIVE_ERROR) || defined(AMA_MEMBER) || defined(AMA_MEMBER_ORDINAL)\n"
                "#error Native context escaped into the assembly consumer\n#endif\n"
                "typedef AmaHostResult ExecutionSignature(AmaProjectHost *, AmaAccessMode);\n"
                "_Static_assert(_Generic(&ama_project_execute, ExecutionSignature *: 1, default: 0), \"Context execution signature disagrees\");\n", output);
            int closed = fclose(output);
            assert(written != EOF && closed == 0);
            compile_translation("generated/agreement.c", optimizations[profile]);
        }
    }
    end_assembly();
}
