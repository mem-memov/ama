/**
 * @file
 * @brief Emit machine-readable native compilation arguments without shell quoting inference.
 *
 * [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06)
 * The supported UTF-8 configuration supplies working directory, includes, flags and file paths as a JSON argument array.
 */
#include <string.h>
#include "native/scaffold/internal.h"

bool scaffold_json(FILE *output, const char *text) {
    if (fputc('"', output) == EOF) {
        return false;
    }
    for (const unsigned char *cursor = (const unsigned char *)text; *cursor != 0; cursor++) {
        unsigned char byte = *cursor;
        if (byte == '"' || byte == '\\') {
            if (fputc('\\', output) == EOF || fputc(byte, output) == EOF) {
                return false;
            }
        } else if (byte < 32) {
            if (fprintf(output, "\\u%04x", (unsigned)byte) < 0) {
                return false;
            }
        } else if (fputc(byte, output) == EOF) {
            return false;
        }
    }
    return fputc('"', output) != EOF;
}

/** Append one independently JSON-escaped argv string; never combine flags into a shell command.
 * Component contract: [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06). */
static bool argument(FILE *output, const char *text) {
    return fputs(", ", output) != EOF && scaffold_json(output, text);
}

/** Emit one compilation database record with supported UTF-8 cwd/file/compiler/flags. Borrow the model only during synchronous emission.
 * Component contract: [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06). */
static bool command(NativeScaffold *scaffold, FILE *output, const char *path, bool *first) {
    if ((!*first && fputs(",\n", output) == EOF)
        || fputs("  {\"directory\": ", output) == EOF || !scaffold_json(output, scaffold->working_directory)
        || fputs(", \"file\": ", output) == EOF || !scaffold_json(output, path)
        || fputs(", \"arguments\": [", output) == EOF
        || !scaffold_json(output, scaffold->options.compiler != NULL ? scaffold->options.compiler : "gcc")) {
        return false;
    }
    *first = false;
    const char *flags[] = {"-std=c11", "-Wall", "-Wextra", "-pedantic", "-I", scaffold->sdk,
        "-I", scaffold->directory, "-UNDEBUG", "-fsyntax-only"};
    for (size_t index = 0; index < sizeof(flags) / sizeof(flags[0]); index++) {
        if (!argument(output, flags[index])) {
            return false;
        }
    }
    for (size_t index = 0; index < scaffold->options.flag_count; index++) {
        if (!argument(output, scaffold->options.flags[index])) {
            return false;
        }
    }
    return argument(output, path) && fputs("]}", output) != EOF;
}

bool scaffold_emit_tooling(NativeScaffold *scaffold, FILE *output, const ScaffoldType *selected) {
    bool first = true;
    if (fputs("[\n", output) == EOF) {
        return false;
    }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
            && !command(scaffold, output, type->body_path, &first)) {
            return false;
        }
    }
    const char *names[] = {selected->template_name, selected->test_name};
    for (size_t index = 0; index < 2; index++) {
        const char *path = scaffold_join(scaffold, scaffold->directory, "/", names[index]);
        if (path == NULL || !command(scaffold, output, path, &first)) {
            return false;
        }
    }
    return fputs("\n]\n", output) != EOF;
}
