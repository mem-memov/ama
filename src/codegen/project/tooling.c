/**
 * @file
 * @brief Emit a machine-readable argv record for the project launcher.
 *
 * [CABI-06](../../../spec/implementation/native-interface.md#cabi-06)
 * Supported UTF-8 flags/paths are independently JSON escaped; there is no reconstructed shell command.
 */
#include "codegen/internal.h"

/**
 * Write a supported UTF-8 JSON compilation database argv for the launcher, explicit
 * flags/includes and non-LTO compilation. The output is informational tooling, not shell text.
 */
bool codegen_emit_tooling(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    const char *path = scaffold_join(scaffold, scaffold->directory, "/", "launcher.c");
    if (path == NULL || fputs("[{\"directory\":", output) == EOF || !scaffold_json(output, scaffold->working_directory)
        || fputs(",\"file\":", output) == EOF || !scaffold_json(output, path)
        || fputs(",\"arguments\":[", output) == EOF
        || !scaffold_json(output, scaffold->options.compiler == NULL ? "gcc" : scaffold->options.compiler)) { return false; }
    for (size_t index = 0; index < scaffold->options.flag_count; index++) {
        if (fputc(',', output) == EOF || !scaffold_json(output, scaffold->options.flags[index])) { return false; }
    }
    const char *flags[] = {"-std=c11", "-O2", "-Wall", "-Wextra", "-fno-lto", "-fno-whole-program", "-I", scaffold->directory, "-I", scaffold->sdk, "-c"};
    for (size_t index = 0; index < sizeof(flags) / sizeof(flags[0]); index++) {
        if (fputc(',', output) == EOF || !scaffold_json(output, flags[index])) { return false; }
    }
    return fputc(',', output) != EOF && scaffold_json(output, path) && fputs("]}]\n", output) != EOF;
}
