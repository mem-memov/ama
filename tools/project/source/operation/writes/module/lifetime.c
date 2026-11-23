/** @file
 * @brief Retire module alias parser, normalized scope, edit and result ownership after every prefix.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03)
 * keeps borrowed source bytes separate from owned transformation scratch.
 */
#include "module.h"
#include <string.h>
void source_operation_module_write_init(SourceOperationModuleWrite *write, SourceAllocator allocator) {
    memset(write, 0, sizeof(*write)); source_arena_init(&write->arena, allocator);
    source_manager_init(&write->texts, allocator); configuration_document_init(&write->document, allocator);
}
void source_operation_module_write_destroy(SourceOperationModuleWrite *write) {
    if (write->edits_initialized) { source_text_edits_destroy(&write->edits); }
    source_arena_destroy(&write->arena); configuration_document_destroy(&write->document);
    source_manager_destroy(&write->texts); memset(write, 0, sizeof(*write));
}
