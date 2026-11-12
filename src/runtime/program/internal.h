/**
 * @file
 * @brief Declare private shared construction and recipe resolution interfaces.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * Borrow resolved descriptors and values; the caller owns scratch and all persistent partial construction cleanup.
 */
#ifndef AMA_RUNTIME_PROGRAM_INTERNAL_H
#define AMA_RUNTIME_PROGRAM_INTERNAL_H
#include "program.h"
#include "runtime/lifecycle/errors.h"

/** Resolve each declared initializer once in source order into owned constructor recipes or copied reference handles. Output recipes borrow sources and the arena; partial scratch remains caller-owned. */
AmaStatus ama_program_resolve_recipes(AmaProgram *, SourceArena *, AmaHandle,
    const AmaConstantValue *, const AmaProgramSource *, size_t, AmaFieldKind, AmaFieldRecipe *);
/** Consume native credit then reverse initialized child/field prefixes using parent links. Release all storage without allocation; secondary failures mark terminal state and preserve the primary status. */
AmaStatus ama_program_constant_cleanup(AmaProgram *, AmaConstantValue *, AmaStatus);
/** Use arena parent cursors to construct child fields before native payload without C recursion. On failure report and clean the partial root before freeing scratch. */
AmaStatus ama_program_constant_build(AmaProgram *, AmaConstantValue *, uint64_t,
    const AmaProgramSource *, const AmaConstantValue *);
#endif
