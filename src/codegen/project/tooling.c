/**
 * @file
 * @brief Emit a machine-readable argv record for the project launcher.
 *
 * [CABI-06](../../../spec/implementation/native-interface.md#cabi-06)
 * Supported UTF-8 flags/paths are independently JSON escaped; there is no reconstructed shell command.
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04) requires the
 * same mandatory profile used by actual compilation, following user options.
 */
#include "codegen/internal.h"
#include "native/toolchain/profile.h"

/**
 * Write a supported UTF-8 JSON compilation database argv for the launcher, explicit
 * flags/includes and non-LTO compilation. The staged entry artifact identifies the
 * same unit as actual private compilation. This record is informational tooling,
 * not shell text.
 * Component contract: [CABI-06](../../../spec/implementation/native-interface.md#cabi-06), [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
bool codegen_emit_tooling(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    const char *path = scaffold->entry_artifact->path;
    if (path == NULL || fputs("[{\"directory\":", output) == EOF || !scaffold_json(output, scaffold->working_directory)
        || fputs(",\"file\":", output) == EOF || !scaffold_json(output, path)
        || fputs(",\"arguments\":[", output) == EOF
        || !scaffold_json(output, scaffold->options.compiler == NULL ? "gcc" : scaffold->options.compiler)) { return false; }
    for (size_t index = 0; index < scaffold->options.flag_count; index++) {
        if (fputc(',', output) == EOF || !scaffold_json(output, scaffold->options.flags[index])) { return false; }
    }
    const char *flags[] = {AMA_TOOLCHAIN_REQUIRED_FLAGS, "-O2", "-I",
        scaffold->directory, "-I", scaffold->sdk, "-c"};
    for (size_t index = 0; index < sizeof(flags) / sizeof(flags[0]); index++) {
        if (fputc(',', output) == EOF || !scaffold_json(output, flags[index])) { return false; }
    }
    return fputc(',', output) != EOF && scaffold_json(output, path) && fputs("]}]\n", output) != EOF;
}
