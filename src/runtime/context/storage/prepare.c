/**
 * @file
 * @brief Allocate each context's descriptors and constant owners before private construction.
 *
 * [RT-01](../../../../spec/implementation/runtime.md#rt-01) forbids sharing mutable pool or
 * constant lifecycle metadata; [CONST-02](../../../../spec/language/declarations.md#const-02)
 * initializes shared values before any explicit instance/body can be requested.
 * [ERROR-03](../../../../spec/implementation/diagnostics.md#error-03) requires
 * checked transport before publishing a context diagnostic or invoking its reporter.
 */
#include <string.h>
#include "../internal.h"

/** Record the first context-local failure before synchronous reporting. Other contexts own
 * different status/program records. The callback receives no implicit execution authority.
 */
static void context_report(void *owner, AmaStatus status) {
    AmaContext *context = owner;
    status = ama_status_checked(status);
    if (status.code == AMA_OK) { return; }
    context->world.failed = true;
    context->control.phase = AMA_RUNTIME_TERMINATED;
    context->status = ama_cleanup_primary(context->status, status);
    if (context->report != NULL) { context->report(context->report_context, status); }
    else { ama_program_report(&context->program, status); }
}

/** Allocate one checked zero-initialized arena array. Existing scratch remains owned on failure;
 * empty arrays need no allocation. This helper serves only context descriptor construction.
 */
static void *context_array(SourceArena *arena, size_t count, size_t width, AmaStatus *status) {
    if (status->code != AMA_OK || count == 0) { return NULL; }
    if (count > SIZE_MAX / width) {
        *status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
        return NULL;
    }
    void *memory = source_arena_allocate(arena, count * width);
    if (memory == NULL) { *status = ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    else { memset(memory, 0, count * width); }
    return memory;
}

/** Install all cleanup-visible array owners before the first type validation or native hook.
 * Every constant storage pointer is valid even when only an earlier construction prefix exists.
 */
static AmaStatus context_arrays(AmaContext *context, const AmaContextDefinition *definition,
    AmaProgramType **mutable_types) {
    AmaStatus status = ama_status_success();
    AmaObjectType *types = context_array(&context->arena, definition->type_count, sizeof(*types), &status);
    AmaObjectType **registry = context_array(&context->arena, definition->type_count, sizeof(*registry), &status);
    AmaProgramType *program_types = context_array(&context->arena, definition->type_count, sizeof(*program_types), &status);
    AmaConstantStorage *storage = context_array(&context->arena, definition->constant_count, sizeof(*storage), &status);
    AmaProgramConstant *constants = context_array(&context->arena, definition->constant_count, sizeof(*constants), &status);
    if (status.code != AMA_OK) { return status; }
    for (size_t index = 0; index < definition->type_count; index++) {
        registry[index] = &types[index];
        program_types[index].object = &types[index];
    }
    for (size_t index = 0; index < definition->constant_count; index++) {
        const AmaContextConstant *source = &definition->constants[index];
        constants[index] = (AmaProgramConstant){.storage = &storage[index], .type_id = source->type_id,
            .list = source->list, .initializers = source->initializers,
            .initializer_count = source->initializer_count, .operation = source->operation};
    }
    context->world.types = registry;
    context->world.type_count = definition->type_count;
    context->program.types = program_types;
    context->program.type_count = definition->type_count;
    context->program.constants = constants;
    context->program.constant_count = definition->constant_count;
    *mutable_types = program_types;
    return ama_status_success();
}

/** Validate/initialize each private nominal pool and bind declaration resolution to this
 * context's program. No pool capacity or implicit root instance is constructed here.
 */
static AmaStatus context_types(AmaContext *context, const AmaContextDefinition *definition,
    AmaProgramType *program_types) {
    for (size_t index = 0; index < definition->type_count; index++) {
        const AmaContextType *source = &definition->types[index];
        AmaObjectType *type = context->world.types[index];
        AmaStatus status = ama_object_type_init_aligned(type, context->world.allocator,
            source->type_id, source->size, source->alignment, source->slots_per_block,
            source->generation_limit, source->fields, source->field_count,
            source->native_offset, source->hooks);
        if (status.code != AMA_OK) { return status; }
        type->resolve_field = ama_program_field_recipe;
        type->field_context = &context->program;
        type->source_operation = source->operation;
        AmaProgramType *program = &program_types[index];
        *program = (AmaProgramType){.object = type, .operation = source->operation,
            .fields = source->declarations, .field_count = source->field_count,
            .condition = source->condition, .index = source->index};
    }
    return ama_status_success();
}

/** Prepare safely destructible private ownership, then construct constants exactly once.
 * Publication is a registry operation; this phase neither issues a host token nor executes code.
 */
AmaStatus ama_context_prepare(AmaContext *context, const AmaContextDefinition *definition,
    SourceAllocator allocator) {
    atomic_init(&context->active, false);
    source_arena_init(&context->arena, allocator);
    context->report = definition->report;
    context->report_context = definition->report_context;
    context->world.allocator = allocator;
    context->world.report = context_report;
    context->world.report_context = context;
    context->program.world = &context->world;
    context->program.locations = definition->locations;
    context->program.location_count = definition->location_count;
    context->program.initialization_operation = definition->initialization_operation;
    AmaProgramType *program_types = NULL;
    AmaStatus status = context_arrays(context, definition, &program_types);
    if (status.code == AMA_OK) { status = context_types(context, definition, program_types); }
    if (status.code == AMA_OK) { status = ama_program_constants_initialize(&context->program); }
    return status;
}
