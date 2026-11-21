/** @file
 * @brief Encode captured local configuration presence and bytes for exact invalidation.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) makes module.ini optional;
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) distinguishes
 * creation/removal from unchanged empty input without rereading physical files.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"

/** Use a one-byte tagged byte sequence, releasing temporary storage on every return.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) compares exact
 * immutable payloads, so no two presence/content observations share an encoding.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) supplies the original key.
 * O(B) temporary bytes/time beyond graph admission; graph-owned copies survive
 * publication or failure under the existing ProjectInputs destructor.
 */
SourceStatus project_input_aliases(ProjectInputs *inputs, const ModuleSource *source,
    ProjectInput *consumer) {
    if (!source->configuration_loaded || source->configuration_path == NULL) {
        return SOURCE_INVALID_ARGUMENT;
    }
    size_t length = source->configuration == NULL ? 0 : source_length(source->configuration);
    if (length == SIZE_MAX) { return SOURCE_LIMIT; }
    SourceAllocator allocator = inputs->arena.allocator;
    unsigned char *bytes = allocator.allocate(allocator.context, length + 1);
    if (bytes == NULL) { return SOURCE_OUT_OF_MEMORY; }
    bytes[0] = source->configuration != NULL;
    if (length != 0) { memcpy(bytes + 1, source_bytes(source->configuration), length); }
    ProjectInput *node = NULL;
    SourceStatus status = project_inputs_put(inputs, PROJECT_INPUT_CONFIGURATION,
        source->configuration_path, bytes, length + 1, &node);
    allocator.release(allocator.context, bytes);
    return status == SOURCE_OK ? project_inputs_depend(inputs, consumer, node) : status;
}
