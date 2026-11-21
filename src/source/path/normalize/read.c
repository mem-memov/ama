/**
 * @file
 * @brief Admit complete raw path syntax and retain its root/alias/component boundaries.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) distinguishes
 * root prefixes before reduction; [PKG-04](../../../../spec/project/packages.md#pkg-04)
 * ends a complete alias name only at its first slash or operand end.
 */
#include "internal.h"
#include "source/path/syntax.h"

ModulePathStatus path_normalization_read(SourceSpan expression, ModulePathInput *output, SourceIssue *issue) {
    if (!source_span_valid(expression)) {
        return path_normalization_failure(issue, (SourceSpan){0}, MODULE_PATH_INVALID,
            "Invalid path source span", "an original bounded path operand");
    }
    const unsigned char *bytes = source_bytes(expression.source) + expression.start;
    size_t length = expression.end - expression.start;
    if (!module_path_syntax(bytes, length)) {
        return path_normalization_failure(issue, expression, MODULE_PATH_INVALID,
            "Invalid path spelling", "a complete absolute, relative or alias path");
    }
    ModulePathInput input = {0};
    size_t start = 0;
    if (bytes[0] == '~') {
        input.aliased = true;
        while (start < length && bytes[start] != '/') { start++; }
        input.alias = (SourceSpan){expression.source, expression.start, expression.start + start};
        if (start < length) { start++; }
    } else if (bytes[0] == '/') {
        input.root = length > 1 && bytes[1] == '/' ? MODULE_ROOT_STANDARD : MODULE_ROOT_PROJECT;
        start = input.root == MODULE_ROOT_STANDARD ? 2 : 1;
    } else { input.relative = true; }
    input.fragment = (ModulePathFragment){bytes + start, length - start,
        {expression.source, expression.start + start, expression.end}};
    *output = input;
    return MODULE_PATH_OK;
}
