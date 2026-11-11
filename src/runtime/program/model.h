#ifndef AMA_RUNTIME_PROGRAM_MODEL_H
#define AMA_RUNTIME_PROGRAM_MODEL_H

#include "runtime/objects/objects.h"

typedef enum { AMA_SOURCE_LOCAL, AMA_SOURCE_SHARED, AMA_SOURCE_LITERAL } AmaProgramSourceKind;
typedef struct AmaProgramAccess {
    size_t field;
    const struct AmaProgramAccess *index;
    bool shared;
} AmaProgramAccess;
typedef struct {
    AmaProgramSourceKind kind;
    const AmaProgramAccess *access;
    size_t shared;
    uint64_t type_id;
    AmaByteView literal;
    bool acquire;
    bool continuation;
} AmaProgramSource;
typedef enum { AMA_PROGRAM_PARAMETER, AMA_PROGRAM_RESULT, AMA_PROGRAM_STATE } AmaProgramRole;
typedef struct {
    AmaProgramRole role;
    size_t position;
    const AmaProgramSource *initializers;
    size_t initializer_count;
} AmaProgramField;
typedef struct {
    AmaObjectType *object;
    const AmaProgramField *fields;
    size_t field_count;
    AmaStatus (*condition)(const void *, bool *);
    AmaStatus (*index)(const void *, size_t *);
} AmaProgramType;
#endif
