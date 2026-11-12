/**
 * @file
 * @brief Resolve exact registered program types without creating instances.
 *
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01)
 * requires nominal type identity, never inferred payload layout.
 */
#include "internal.h"

/** Find the exact nominal type in the borrowed registry, or return null; no allocation or state changes. */
const AmaProgramType *ama_program_type(const AmaProgram *program, uint64_t id) {
    for (size_t index = 0; index < program->type_count; index++) {
        const AmaProgramType *type = &program->types[index];
        if (type->object != NULL && type->object->pool.type_id == id) { return type; }
    }
    return NULL;
}
