/**
 * @file
 * @brief Serialize complete ordered source-root settings without ambiguous concatenation.
 * [DEP-02](../../../../spec/implementation/compiler.md#dep-02) makes root kinds and exact exclusions part of source resolution authority.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"
#include "source/io/buffer/buffer.h"
_Static_assert(SIZE_MAX <= UINT64_MAX, "Root inventory counts fit their uint64 encoding");

/** Append one fixed-width big-endian count with checked buffer arithmetic. */
static SourceStatus number(ProjectInputs *inputs, SourceBuffer *buffer, size_t value) {
    unsigned char bytes[8];
    for (size_t index = 0; index < 8; index++) {
        bytes[7 - index] = (unsigned char)((uint64_t)value >> (index * 8));
    }
    return source_buffer_append(inputs->arena.allocator, buffer, bytes, sizeof(bytes));
}
/** Append one non-null original string with its length prefix; no trimming or path reinterpretation. */
static SourceStatus text(ProjectInputs *inputs, SourceBuffer *buffer, const char *value) {
    if (value == NULL) { return SOURCE_INVALID_ARGUMENT; }
    size_t length = strlen(value);
    SourceStatus status = number(inputs, buffer, length);
    return status == SOURCE_OK ? source_buffer_append(inputs->arena.allocator, buffer,
        (const unsigned char *)value, length) : status;
}
/** Encode kind, physical root and the complete ordered exclusion list for one configured root. */
static SourceStatus root(ProjectInputs *inputs, SourceBuffer *buffer, const ModuleRoot *value) {
    if ((value->kind != MODULE_ROOT_PROJECT && value->kind != MODULE_ROOT_STANDARD)
        || (value->excluded_count != 0 && value->excluded == NULL)) { return SOURCE_INVALID_ARGUMENT; }
    SourceStatus status = number(inputs, buffer, value->kind);
    if (status == SOURCE_OK) { status = text(inputs, buffer, value->path); }
    if (status == SOURCE_OK) { status = number(inputs, buffer, value->excluded_count); }
    for (size_t index = 0; status == SOURCE_OK && index < value->excluded_count; index++) {
        status = text(inputs, buffer, value->excluded[index]);
    }
    return status;
}
SourceStatus project_input_roots(ProjectInputs *inputs, const ModuleRoot *roots,
    size_t count, ProjectInput **output) {
    if (count != 0 && roots == NULL) { return SOURCE_INVALID_ARGUMENT; }
    SourceBuffer buffer = {0};
    SourceStatus status = number(inputs, &buffer, count);
    for (size_t index = 0; status == SOURCE_OK && index < count; index++) {
        status = root(inputs, &buffer, &roots[index]);
    }
    if (status == SOURCE_OK) {
        status = project_inputs_put(inputs, PROJECT_INPUT_CONFIGURATION, "root inventory",
            buffer.bytes, buffer.length, output);
    }
    inputs->arena.allocator.release(inputs->arena.allocator.context, buffer.bytes);
    return status;
}
