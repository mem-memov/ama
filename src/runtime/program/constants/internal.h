#ifndef AMA_RUNTIME_CONSTANT_BUILD_INTERNAL_H
#define AMA_RUNTIME_CONSTANT_BUILD_INTERNAL_H
#include "runtime/program/internal.h"

typedef struct AmaConstantBuildWork {
    struct AmaConstantBuildWork *parent;
    AmaConstantValue *value;
    const AmaConstantValue *source;
    const AmaProgramSource *initializer;
    uint64_t operation;
    size_t field;
    size_t element;
} AmaConstantBuildWork;

AmaStatus ama_constant_begin(AmaProgram *, SourceArena *, AmaConstantValue *, uint64_t,
    const AmaProgramSource *, const AmaConstantValue *, AmaConstantBuildWork *, AmaConstantBuildWork **);
AmaStatus ama_constant_field(AmaProgram *, AmaConstantBuildWork *);
AmaStatus ama_constant_payload(AmaProgram *, AmaConstantBuildWork *);
#endif
