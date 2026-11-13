/**
 * @file
 * @brief Declare the arena-owned iterative constant construction cursor.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * Work records borrow persistent constant values; releasing scratch does not destroy completed constant storage.
 */
#ifndef AMA_RUNTIME_CONSTANT_BUILD_INTERNAL_H
#define AMA_RUNTIME_CONSTANT_BUILD_INTERNAL_H
#include "runtime/program/internal.h"

/** Arena-owned iterative constant cursor; borrow persistent value/source/initializer and parent, tracking current field/element and failure operation. */
typedef struct AmaConstantBuildWork {
    struct AmaConstantBuildWork *parent;
    AmaConstantValue *value;
    const AmaConstantValue *source;
    const AmaProgramSource *initializer;
    uint64_t operation;
    size_t field;
    size_t element;
} AmaConstantBuildWork;

/** Allocate one arena cursor, resolve a nonliteral shared source once, check exact type and register zeroed field storage. Partial constant storage remains owned by cleanup on failure. */
AmaStatus ama_constant_begin(AmaProgram *, SourceArena *, AmaConstantValue *, uint64_t,
    const AmaProgramSource *, const AmaConstantValue *, AmaConstantBuildWork *, AmaConstantBuildWork **);
/** Allocate the declared field array once with checked count arithmetic. References require no owned child payload; evaluate explicit reference sources without introducing mutable constant handles. */
AmaStatus ama_constant_field(AmaProgram *, AmaConstantBuildWork *);
/** Record aligned allocation and invoke exactly clone, literal or default. Raw code
 * zero creates native_live before checked failure; nonzero construction owns its
 * own partial native cleanup. Caller rollback consumes completed payload credits.
 */
AmaStatus ama_constant_payload(AmaProgram *, AmaConstantBuildWork *);
#endif
