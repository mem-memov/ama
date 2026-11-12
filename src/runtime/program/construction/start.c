/**
 * @file
 * @brief Initialize shared storage before the explicit root and coordinate reverse shutdown.
 *
 * [RT-11](../../../../spec/implementation/runtime.md#rt-11)
 * Stop destroys the root, constants and pool allocations while retaining the primary failure; it does not dispatch module bodies.
 */
#include "runtime/program/program.h"
#include "runtime/lifecycle/errors.h"

/** Install borrowed type recipe callbacks, initialize constants, then construct exactly one explicit root. Root output starts empty; caller uses stop to clean completed prefixes after failure. */
AmaStatus ama_program_start(AmaProgram *program, uint64_t root_type, AmaHandle *root) {
    *root = (AmaHandle){0};
    for (size_t index = 0; index < program->type_count; index++) {
        AmaObjectType *object = program->types[index].object;
        if (object == NULL) { continue; }
        object->resolve_field = ama_program_field_recipe;
        object->field_context = program;
        object->source_operation = program->types[index].operation;
    }
    AmaStatus status = ama_program_constants_initialize(program);
    if (status.code != AMA_OK) { return status; }
    return ama_object_create(program->world, root_type, NULL, root);
}

/** Destroy a present root, reverse-clean constants and dispose finalized pools. Preserve the initial failure and mark failed execution terminal before cleanup. */
AmaStatus ama_program_stop(AmaProgram *program, AmaHandle root, AmaStatus primary) {
    if (primary.code != AMA_OK) { program->world->failed = true; }
    if (root.generation != 0) {
        AmaStatus status = ama_object_destroy(program->world, root);
        primary = ama_cleanup_primary(primary, status);
    }
    primary = ama_program_constants_destroy(program, primary);
    for (size_t index = 0; index < program->world->type_count; index++) {
        AmaObjectType *type = program->world->types[index];
        if (type != NULL && type->pool.allocator.release != NULL) { ama_pool_dispose(&type->pool); }
    }
    return primary;
}
