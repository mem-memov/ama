#ifndef AMA_PROGRAM_TRANSFER_INTERNAL_H
#define AMA_PROGRAM_TRANSFER_INTERNAL_H
#include "runtime/program/internal.h"
#include "runtime/objects/internal.h"
#include "runtime/program/calls/call.h"
AmaStatus ama_program_store(AmaProgram *, AmaProgramValue *, const AmaProgramValue *,
    const AmaFieldRecipe *, AmaObjectOperation *);
AmaStatus ama_program_store_list(AmaProgram *, AmaProgramValue *, const AmaProgramValue *,
    const AmaFieldRecipe *, AmaObjectOperation *);
AmaStatus ama_program_value_recipe(AmaProgram *, SourceArena *, const AmaProgramValue *, bool, AmaRecipe *);
#endif
