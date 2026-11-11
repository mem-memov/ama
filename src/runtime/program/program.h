#ifndef AMA_RUNTIME_PROGRAM_H
#define AMA_RUNTIME_PROGRAM_H
#include "constants.h"

typedef AmaConstantField AmaConstantStorage;
typedef struct {
    AmaConstantStorage *storage;
    uint64_t type_id;
    bool list;
    const AmaProgramSource *initializers;
    size_t initializer_count;
    uint64_t operation;
} AmaProgramConstant;
typedef struct {
    AmaWorld *world;
    const AmaProgramType *types;
    size_t type_count;
    const AmaProgramConstant *constants;
    size_t constant_count;
    AmaHandle current;
    uint64_t initialization_operation;
    const struct AmaProgramLocation *locations;
    size_t location_count;
    bool failure_reported;
} AmaProgram;
typedef struct {
    uint64_t type_id;
    AmaFieldKind form;
    AmaHandle handle;
    AmaHandle *location;
    AmaHandleList *list;
    const AmaConstantValue *constant;
    const AmaConstantStorage *shared_list;
    AmaHandle container;
    bool private_access;
    bool continuation;
} AmaProgramValue;

const AmaProgramType *ama_program_type(const AmaProgram *, uint64_t);
AmaStatus ama_program_select(AmaProgram *, AmaHandle, const AmaProgramAccess *, bool, AmaProgramValue *);
AmaStatus ama_program_select_shared(AmaProgram *, const AmaConstantValue *, const AmaProgramAccess *, AmaProgramValue *);
AmaStatus ama_program_evaluate(AmaProgram *, AmaHandle, const AmaConstantValue *, const AmaProgramSource *, bool, AmaProgramValue *);
AmaStatus ama_program_payload(AmaProgram *, const AmaProgramValue *, const void **);
AmaStatus ama_program_field_recipe(AmaWorld *, SourceArena *, AmaHandle, size_t, AmaFieldRecipe *, void *);
AmaStatus ama_program_constants_initialize(AmaProgram *);
AmaStatus ama_program_constants_destroy(AmaProgram *, AmaStatus);
AmaStatus ama_program_constant_recipe(AmaProgram *, SourceArena *, const AmaConstantValue *, AmaRecipe *);
AmaStatus ama_program_start(AmaProgram *, uint64_t, AmaHandle *);
AmaStatus ama_program_stop(AmaProgram *, AmaHandle, AmaStatus);
AmaStatus ama_program_select_target(AmaProgram *, AmaHandle, const AmaProgramAccess *, AmaProgramValue *, bool *);
#endif
