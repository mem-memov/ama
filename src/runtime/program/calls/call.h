#ifndef AMA_PROGRAM_CALL_H
#define AMA_PROGRAM_CALL_H
#include "runtime/program/program.h"

typedef struct {
    size_t field;
    uint64_t operation;
    AmaProgramSource source;
} AmaProgramInput;
typedef struct {
    const AmaProgramAccess *target;
    AmaAccessMode mode;
    const AmaProgramInput *inputs;
    size_t input_count;
    const uint64_t *installation_operations;
} AmaProgramInvocation;
AmaStatus ama_program_acquire(AmaProgram *, AmaHandle, const AmaProgramInvocation *, bool *);
AmaStatus ama_program_prepare(AmaProgram *, AmaHandle, const AmaProgramInvocation *);
AmaStatus ama_program_release(AmaProgram *, AmaHandle);
AmaStatus ama_program_connect(AmaProgram *, AmaHandle *, uint64_t, uint64_t);
AmaStatus ama_program_complete(AmaProgram *, AmaHandle *, uint64_t *);
AmaStatus ama_program_cell(AmaProgram *, AmaHandle, AmaPoolCell **);
AmaStatus ama_program_assign(AmaProgram *, AmaHandle, const AmaProgramAccess *, const AmaProgramSource *);
AmaStatus ama_program_transfer(AmaProgram *, AmaHandle, const AmaProgramAccess *, const AmaProgramValue *);
AmaStatus ama_program_condition(AmaProgram *, AmaHandle, const AmaProgramAccess *, bool *);
#endif
