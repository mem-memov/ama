#include "runtime/program/internal.h"

AmaStatus ama_program_constant_cleanup(AmaProgram *program, AmaConstantValue *root, AmaStatus primary) {
    AmaConstantValue *value = root;
    while (value != NULL) {
        if (value->type == NULL) { return primary; }
        if (value->native_live) {
            value->native_live = false;
            AmaStatus status = value->type->object->hooks.destroy(value->payload);
            if (status.code != AMA_OK) {
                if (program->world->report != NULL) { program->world->report(program->world->report_context, status); }
                program->world->failed = true;
            }
            primary = ama_cleanup_primary(primary, status);
        }
        if (value->remaining_fields != 0 && value->fields != NULL) {
            size_t field_index = value->remaining_fields - 1;
            AmaConstantField *field = &value->fields[field_index];
            if (field->count != 0 && field->items != NULL) {
                AmaConstantValue *child = &field->items[--field->count];
                if (child->type != NULL) { value = child; }
                continue;
            }
            program->world->allocator.release(program->world->allocator.context, field->items);
            *field = (AmaConstantField){0};
            value->remaining_fields--;
            continue;
        }
        AmaConstantValue *parent = value->parent;
        program->world->allocator.release(program->world->allocator.context, value->allocation);
        program->world->allocator.release(program->world->allocator.context, value->fields);
        bool done = value == root;
        *value = (AmaConstantValue){0};
        if (done) { return primary; }
        value = parent;
    }
    return primary;
}

AmaStatus ama_program_constants_destroy(AmaProgram *program, AmaStatus primary) {
    for (size_t declaration = program->constant_count; declaration != 0;) {
        AmaConstantStorage *storage = program->constants[--declaration].storage;
        while (storage->count != 0 && storage->items != NULL) {
            primary = ama_program_constant_cleanup(program, &storage->items[--storage->count], primary);
        }
        program->world->allocator.release(program->world->allocator.context, storage->items);
        *storage = (AmaConstantStorage){0};
    }
    return primary;
}
