/** @file
 * @brief Recognize exact bounded debugger commands while draining every invalid line.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) forbids silent truncation or guessed input interpretation.
 */
#include <string.h>
#include "../internal.h"
/** Seven bytes is the maximum accepted spelling, not an arbitrary input limit:
 * longer lines are fully consumed and rejected, never treated as valid prefixes.
 * Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
AmaDebugClientCommand ama_debug_client_read(FILE *input) {
    char text[8];
    size_t length = 0;
    bool invalid = false;
    int byte;
    while ((byte = fgetc(input)) != EOF && byte != '\n') {
        if (length == sizeof(text) - 1 || byte == 0) {
            invalid = true;
        } else {
            text[length++] = (char)byte;
        }
    }
    if (ferror(input)) { return AMA_DEBUG_CLIENT_IO; }
    if (byte == EOF && length == 0 && !invalid) { return AMA_DEBUG_CLIENT_END; }
    if (invalid) { return AMA_DEBUG_CLIENT_INVALID; }
    text[length] = 0;
    static const struct { const char *name; AmaDebugClientCommand command; } commands[] = {
        {"run", AMA_DEBUG_CLIENT_RUN}, {"resume", AMA_DEBUG_CLIENT_RESUME},
        {"inspect", AMA_DEBUG_CLIENT_INSPECT}, {"dispose", AMA_DEBUG_CLIENT_DISPOSE},
        {"quit", AMA_DEBUG_CLIENT_QUIT}
    };
    for (size_t index = 0; index < sizeof(commands) / sizeof(*commands); index++) {
        if (strcmp(text, commands[index].name) == 0) { return commands[index].command; }
    }
    return AMA_DEBUG_CLIENT_INVALID;
}
