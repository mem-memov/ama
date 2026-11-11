#ifndef AMA_RUNTIME_PROGRAM_INTERNAL_H
#define AMA_RUNTIME_PROGRAM_INTERNAL_H
#include "program.h"
#include "runtime/lifecycle/errors.h"

AmaStatus ama_program_resolve_recipes(AmaProgram *, SourceArena *, AmaHandle,
    const AmaConstantValue *, const AmaProgramSource *, size_t, AmaFieldKind, AmaFieldRecipe *);
AmaStatus ama_program_constant_cleanup(AmaProgram *, AmaConstantValue *, AmaStatus);
AmaStatus ama_program_constant_build(AmaProgram *, AmaConstantValue *, uint64_t,
    const AmaProgramSource *, const AmaConstantValue *);
#endif
