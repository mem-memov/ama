/**
 * @file
 * @brief Declare scalar/list store phases and immutable-source recipe conversion.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * The enclosing guarded operation owns protections and scratch; list commit ownership is defined by its separate phase contract.
 */
#ifndef AMA_PROGRAM_TRANSFER_INTERNAL_H
#define AMA_PROGRAM_TRANSFER_INTERNAL_H
#include "runtime/program/internal.h"
#include "runtime/objects/internal.h"
#include "runtime/program/calls/call.h"
struct AmaProgramListService;
/** Transfer a method-direct result to a caller receiver with the exact structural
 * grant. Ordinary transfers always pass null and inherit no list authority.
 * Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
AmaStatus ama_program_transfer_authorized(AmaProgram *, AmaHandle, const AmaProgramAccess *,
    const AmaProgramValue *, const struct AmaProgramListService *);
/** Install one typed scalar or delegate whole-list transfer. Reference assignment copies handles without target acquisition; owning values use staged reconstruction in the caller operation. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
AmaStatus ama_program_store(AmaProgram *, AmaProgramValue *, const AmaProgramValue *,
    const AmaFieldRecipe *, AmaObjectOperation *);
/** Prepare/fix up the complete private forest before commit. Successful commit consumes staging; failed commit retains cleanup ownership as specified by the list phase contract. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
AmaStatus ama_program_store_list(AmaProgram *, AmaProgramValue *, const AmaProgramValue *,
    const AmaFieldRecipe *, AmaObjectOperation *);
/** Require an owned scalar source; produce an immutable-constant recipe or exact handle clone recipe with explicit continuation mode. Arena/source lifetimes cover construction. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
AmaStatus ama_program_value_recipe(AmaProgram *, SourceArena *, const AmaProgramValue *, bool, AmaRecipe *);
#endif
