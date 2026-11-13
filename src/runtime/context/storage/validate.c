/**
 * @file
 * @brief Validate resolved context schema shapes and exact nominal registry membership.
 *
 * [RT-01](../../../../spec/implementation/runtime.md#rt-01) separates descriptors from owned
 * context storage. Reject malformed counts/keys before native construction; no type is inferred.
 */
#include "../internal.h"

/** Find one complete nominal key in a finite schema; no basename or hash substitutes for equality. */
static bool context_has_type(const AmaContextType *types, size_t count, uint64_t identity) {
    for (size_t index = 0; index < count; index++) {
        if (types[index].type_id == identity) { return true; }
    }
    return false;
}

/** Validate all shapes consumed by private descriptor installation. Deeper initializer/graph
 * semantics belong to the already resolved compiler schema, not a second runtime language parser.
 */
AmaStatus ama_context_validate(const AmaContextDefinition *definition) {
    if (definition == NULL || (definition->type_count != 0 && definition->types == NULL)
        || (definition->constant_count != 0 && definition->constants == NULL)
        || (definition->location_count != 0 && definition->locations == NULL)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (definition->type_count > SIZE_MAX / sizeof(AmaObjectType)
        || definition->type_count > SIZE_MAX / sizeof(AmaObjectType *)
        || definition->type_count > SIZE_MAX / sizeof(AmaProgramType)
        || definition->constant_count > SIZE_MAX / sizeof(AmaConstantStorage)
        || definition->constant_count > SIZE_MAX / sizeof(AmaProgramConstant)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    for (size_t index = 0; index < definition->type_count; index++) {
        const AmaContextType *type = &definition->types[index];
        if (type->type_id == 0 || (type->field_count != 0
                && (type->fields == NULL || type->declarations == NULL))) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
        if (type->field_count > SIZE_MAX / sizeof(AmaObjectField)
            || type->field_count > SIZE_MAX / sizeof(AmaProgramField)) {
            return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
        }
        for (size_t previous = 0; previous < index; previous++) {
            if (definition->types[previous].type_id == type->type_id) {
                return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
            }
        }
        for (size_t field = 0; field < type->field_count; field++) {
            const AmaProgramField *declaration = &type->declarations[field];
            if (!context_has_type(definition->types, definition->type_count, type->fields[field].type_id)) {
                return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
            }
            if (declaration->role < AMA_PROGRAM_PARAMETER || declaration->role > AMA_PROGRAM_STATE
                || (declaration->initializer_count != 0 && declaration->initializers == NULL)) {
                return ama_status_make(AMA_INVALID_STATE, NULL, 0);
            }
        }
    }
    for (size_t index = 0; index < definition->constant_count; index++) {
        const AmaContextConstant *constant = &definition->constants[index];
        if (!context_has_type(definition->types, definition->type_count, constant->type_id)) {
            return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
        }
        if ((!constant->list && constant->initializer_count > 1)
            || (constant->initializer_count != 0 && constant->initializers == NULL)) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
    }
    return ama_status_success();
}
