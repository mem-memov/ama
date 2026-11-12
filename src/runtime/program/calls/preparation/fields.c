/**
 * @file
 * @brief Preflight every affected interface field and reject protected-list overlap.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * requires availability checks before the first replacement; body state stays untouched.
 */
#include "internal.h"

AmaStatus ama_call_check_fields(AmaProgram *program, AmaHandle target, const AmaProgramType *type,
    const AmaPreparedInput *inputs, size_t input_count) {
    AmaStatus status = ama_status_success();
    AmaObjectOperation check = {.world = program->world};
    source_arena_init(&check.arena, program->world->allocator);
    for (size_t field = 0; field < type->field_count && status.code == AMA_OK; field++) {
        if (type->fields[field].role == AMA_PROGRAM_STATE) { continue; }
        AmaProgramAccess access = {.field = field};
        AmaProgramValue value = {0};
        status = ama_program_select(program, target, &access, false, &value);
        if (status.code != AMA_OK) { break; }
        for (size_t index = 0; index < input_count; index++) {
            if (inputs[index].input->source.kind == AMA_SOURCE_LITERAL) { continue; }
            if (value.list != NULL && inputs[index].value.list == value.list) { status = ama_status_make(AMA_BUSY, NULL, 0); break; }
        }
        if (status.code != AMA_OK) { break; }
        for (size_t index = 0; index < input_count && status.code == AMA_OK; index++) {
            const AmaProgramValue *source = &inputs[index].value;
            if (source->list == NULL || source->container.generation == 0) { continue; }
            size_t count = value.form == AMA_FIELD_OWNED ? 1 : value.form == AMA_FIELD_OWNING_LIST ? value.list->count : 0;
            for (size_t element = 0; element < count && status.code == AMA_OK; element++) {
                AmaHandle root = value.form == AMA_FIELD_OWNED ? value.handle : value.list->items[element];
                bool overlaps = false;
                status = ama_object_contains(program->world, root, source->container, &overlaps);
                if (status.code == AMA_OK && overlaps) { status = ama_status_make(AMA_BUSY, NULL, 0); }
            }
        }
        if (status.code != AMA_OK) { break; }
        if (value.form == AMA_FIELD_OWNED) { status = ama_object_preflight(&check, value.handle, false, false); }
        else if (value.form == AMA_FIELD_OWNING_LIST) {
            for (size_t element = 0; element < value.list->count && status.code == AMA_OK; element++) {
                status = ama_object_preflight(&check, value.list->items[element], false, false);
            }
        }
    }
    ama_object_unprotect(&check);
    source_arena_destroy(&check.arena);
    return status;
}
