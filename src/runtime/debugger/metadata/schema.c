/** @file
 * @brief Match explicit debugger names/readers to the current immutable program metadata.
 * [NAME-03](../../../../spec/implementation/compiler.md#name-03) preserves original names;
 * [REF-02](../../../../spec/language/values-and-references.md#ref-02) separates component/context authority.
 */
#include "../internal.h"
/** Catalog entries match exact nominal IDs; absent names remain absent instead of guessed from hashes or source rows. */
const AmaDebugType *ama_debug_type(const AmaDebugSession *session, uint64_t type_id) {
    if (session->catalog == NULL) { return NULL; }
    for (size_t index = 0; index < session->catalog->type_count; index++) {
        if (session->catalog->types[index].type_id == type_id) { return &session->catalog->types[index]; }
    }
    return NULL;
}
/** Operation identities may be stored in model order; exact lookup makes no sorted-table assumption. */
const AmaProgramLocation *ama_debug_location(const AmaDebugSession *session, uint64_t operation) {
    if (operation == 0) { return NULL; }
    for (size_t index = 0; index < session->view.program->location_count; index++) {
        const AmaProgramLocation *location = &session->view.program->locations[index];
        if (location->operation == operation) { return location; }
    }
    return NULL;
}
/** Check all extents/roles and duplicate IDs before observers can use a catalog; no payload is examined. */
AmaStatus ama_debug_schema(const AmaDebugSession *session) {
    const AmaProgram *program = session->view.program;
    const AmaDebugCatalog *catalog = session->catalog;
    if (catalog != NULL) {
        if (catalog->type_count > SIZE_MAX/sizeof(*catalog->types)) { return ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); }
        if (catalog->locations != program->locations || catalog->location_count != program->location_count
            || (catalog->type_count != 0 && catalog->types == NULL)
            || catalog->constant_count != program->constant_count || (catalog->constant_count != 0 && catalog->constants == NULL)) { goto invalid; }
        for (size_t index = 0; index < catalog->type_count; index++) {
            const AmaDebugType *type = &catalog->types[index];
            const AmaProgramType *actual = ama_program_type(program,type->type_id);
            if (type->type_id == 0 || actual == NULL || type->name == NULL || type->field_count != actual->field_count
                || (type->field_count != 0 && type->fields == NULL) || type->binding_count != actual->foreach_count
                || (type->binding_count != 0 && type->bindings == NULL)) { goto invalid; }
            for (size_t prior = 0; prior < index; prior++) { if (catalog->types[prior].type_id == type->type_id) { goto invalid; } }
            for (size_t field = 0; field < type->field_count; field++) {
                if (type->fields[field].name == NULL || type->fields[field].operation != actual->fields[field].operation) { goto invalid; }
            }
            for (size_t binding = 0; binding < type->binding_count; binding++) { if (type->bindings[binding] == NULL) { goto invalid; } }
        }
        for (size_t index = 0; index < catalog->constant_count; index++) {
            if (catalog->constants[index].owner == NULL || catalog->constants[index].name == NULL
                || catalog->constants[index].operation != program->constants[index].operation) { goto invalid; }
        }
    }
    for (size_t index = 0; index < session->reader_count; index++) {
        const AmaDebugNativeReader *reader = &session->readers[index];
        const AmaProgramType *type = ama_program_type(program,reader->type_id);
        if (type == NULL || type->object->hooks.initialize == NULL || reader->read == NULL) { goto invalid; }
        for (size_t prior = 0; prior < index; prior++) { if (session->readers[prior].type_id == reader->type_id) { goto invalid; } }
    }
    return ama_status_success();
invalid:
    return ama_status_make(AMA_INVALID_STATE,NULL,0);
}
