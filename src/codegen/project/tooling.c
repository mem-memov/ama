#include "codegen/internal.h"

bool codegen_emit_tooling(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    const char *path = scaffold_join(scaffold, scaffold->directory, "/", "launcher.c");
    if (path == NULL || fputs("[{\"directory\":", output) == EOF || !scaffold_json(output, scaffold->working_directory)
        || fputs(",\"file\":", output) == EOF || !scaffold_json(output, path)
        || fputs(",\"arguments\":[", output) == EOF
        || !scaffold_json(output, scaffold->options.compiler == NULL ? "gcc" : scaffold->options.compiler)) { return false; }
    const char *flags[] = {"-std=c11", "-O2", "-Wall", "-Wextra", "-I", scaffold->sdk, "-I", scaffold->directory};
    for (size_t index = 0; index < sizeof(flags) / sizeof(flags[0]); index++) {
        if (fputc(',', output) == EOF || !scaffold_json(output, flags[index])) { return false; }
    }
    for (size_t index = 0; index < scaffold->options.flag_count; index++) {
        if (fputc(',', output) == EOF || !scaffold_json(output, scaffold->options.flags[index])) { return false; }
    }
    return fputc(',', output) != EOF && scaffold_json(output, path) && fputs("]}]\n", output) != EOF;
}
