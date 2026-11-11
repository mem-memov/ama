#include <stdint.h>
#include <string.h>
#include "directive.h"
#include "source/unicode/unicode.h"

static bool write_filename(FILE *output, const unsigned char *path) {
    for (; *path != 0; path++) {
        unsigned char byte = *path;
        if (byte == '"' || byte == '\\' || byte == '?') {
            if (fputc('\\', output) == EOF || fputc(byte, output) == EOF) {
                return false;
            }
        } else if (byte < 32 || byte == 127) {
            if (fprintf(output, "\\%03o", (unsigned int)byte) < 0) {
                return false;
            }
        } else if (fputc(byte, output) == EOF) {
            return false;
        }
    }
    return true;
}

SourceStatus source_write_line_directive(FILE *output, const SourceText *source, size_t offset) {
    SourcePosition position;
    if (output == NULL || !source_position(source, offset, &position)) {
        return SOURCE_INVALID_ARGUMENT;
    }
    if (position.line > INT32_MAX) {
        return SOURCE_LIMIT;
    }
    const char *path = source_path(source);
    size_t error;
    if (!utf8_validate((const unsigned char *)path, strlen(path), &error)) {
        return SOURCE_INVALID_ARGUMENT;
    }
    return fprintf(output, "#line %zu \"", position.line) >= 0
        && write_filename(output, (const unsigned char *)path)
        && fputs("\"\n", output) != EOF ? SOURCE_OK : SOURCE_IO_ERROR;
}
