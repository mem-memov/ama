/**
 * @file
 * @brief Own input graph storage and close the mutable builder before comparison.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) requires explicit destruction after partial capture.
 */
#include "internal.h"
void project_inputs_init(ProjectInputs *inputs, SourceAllocator allocator) {
    *inputs = (ProjectInputs){0};
    source_arena_init(&inputs->arena, allocator);
    source_manager_init(&inputs->sources, allocator);
}
void project_inputs_destroy(ProjectInputs *inputs) {
    source_arena_destroy(&inputs->arena);
    source_manager_destroy(&inputs->sources);
    *inputs = (ProjectInputs){0};
}
void project_inputs_seal(ProjectInputs *inputs) {
    inputs->sealed = true;
}
