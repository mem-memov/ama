#include "common/source.h"

void emit_line_directive(FILE* output, const char* path, uint32_t line) {
    fprintf(output, "#line %u \"", line);
    for (const unsigned char* byte = (const unsigned char*)path; *byte; byte++) {
        if (*byte == '"' || *byte == '\\') {
            fputc('\\', output);
            fputc(*byte, output);
        } else if (*byte < 32 || *byte == 127 || *byte == '?') {
            /* Fixed-width octal cannot consume a following hexadecimal digit.
                Escaping '?' also prevents legacy C trigraph interpretation.
            */
            fprintf(output, "\\%03o", *byte);
        } else {
            fputc(*byte, output);
        }
    }
    fputs("\"\n", output);
}
