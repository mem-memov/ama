/** @file
 * @brief Retire configuration edit/results after every successful or failed recipe prefix.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03)
 * keeps exact preparation scratch owned until all borrowing plan consumers expire.
 */
#include "configuration.h"
#include <string.h>
void source_operation_configuration_write_init(SourceOperationConfigurationWrite *write, SourceAllocator allocator) {
    memset(write, 0, sizeof(*write)); source_arena_init(&write->arena, allocator);
    source_manager_init(&write->texts, allocator);
}
void source_operation_configuration_write_destroy(SourceOperationConfigurationWrite *write) {
    if (write->edits_initialized) { source_text_edits_destroy(&write->edits); }
    source_manager_destroy(&write->texts); source_arena_destroy(&write->arena); memset(write, 0, sizeof(*write));
}
