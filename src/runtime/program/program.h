/**
 * @file
 * @brief Declare generated-program ownership, value selection and startup boundaries.
 *
 * [RT-05](../../../spec/implementation/runtime.md#rt-05)
 * Operate directly on stable world storage. The runtime/context owner supplies isolated descriptors and an independent native code provider.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) keeps native
 * creator-image retention independent of context-owned Ama storage.
 * [FLOW-05](../../../spec/language/lists.md#flow-05) Selected binding views borrow origin metadata without copying that association into ordinary references.
 */
#ifndef AMA_RUNTIME_PROGRAM_H
#define AMA_RUNTIME_PROGRAM_H
#include "constants.h"
#include "native/abi/code.h"

/** Shared declaration storage uses the same owned value-array lifecycle as a constant field. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
typedef AmaConstantField AmaConstantStorage;
/** Borrowed constant declaration with writable storage owner, exact nominal type, shape and ordered initializer descriptors. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
typedef struct {
    AmaConstantStorage *storage;
    uint64_t type_id;
    bool list;
    const AmaProgramSource *initializers;
    size_t initializer_count;
    uint64_t operation;
} AmaProgramConstant;
/** Borrowed context-local registries/world and synchronous diagnostic state; native code retention is independent of Ama storage. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
typedef struct {
    AmaWorld *world;
    /** Immutable native image provider, independent of every Ama pool/context lifetime. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
    AmaNativeCodeProvider native_code;
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
/** Borrowed selected scalar/list/shared value. Locations expire after owner destruction or list-array mutation; handles alone do not acquire target state. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
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
    /** Indexed reference receivers retain exact occurrence identity through source evaluation. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
    AmaHandleList *membership_list;
    AmaMembership membership;
    /** Borrowed origin cell only for this selected binding view, never copied data. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
    struct AmaProgramForeachBinding *foreach_binding;
} AmaProgramValue;

/** Resolve the exact generated nominal descriptor in a borrowed program registry, or null; no allocation or type-name inference. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
const AmaProgramType *ama_program_type(const AmaProgram *, uint64_t);
/** Resolve a declared instance access to borrowed typed storage under its availability rules. Index protocols run once; output lifetime ends with owner cleanup or list relocation. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_select(AmaProgram *, AmaHandle, const AmaProgramAccess *, bool, AmaProgramValue *);
/** Resolve immutable shared fields without instance acquisition; borrowed constant storage remains valid through program shutdown. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_select_shared(AmaProgram *, const AmaConstantValue *, const AmaProgramAccess *, AmaProgramValue *);
/** Evaluate one resolved local/shared/literal source exactly once; borrow selected values and preserve explicit continuation intent. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_evaluate(AmaProgram *, AmaHandle, const AmaConstantValue *, const AmaProgramSource *, bool, AmaProgramValue *);
/** Resolve a scalar value to borrowed readable native payload, checking runtime availability for instance storage. Lists do not implicitly become scalar payloads. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_payload(AmaProgram *, const AmaProgramValue *, const void **);
/** Adapt one validated field descriptor to initializer resolution and record its exact diagnostic operation. The program is borrowed through the object type callback context. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_field_recipe(AmaWorld *, SourceArena *, AmaHandle, size_t, AmaFieldRecipe *, void *);
/** Build shared declarations/elements in order, recording partial storage for shutdown. Failure stops before later constants or root creation. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_constants_initialize(AmaProgram *);
/** Clean declarations and list elements in reverse order, releasing arrays and resetting storage even when destructors fail. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_constants_destroy(AmaProgram *, AmaStatus);
/** Iteratively create arena-owned field/child clone recipes borrowing immutable shared native payloads. Failures retain scratch for caller bulk release; source lifetime must cover construction. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_constant_recipe(AmaProgram *, SourceArena *, const AmaConstantValue *, AmaRecipe *);
/** Install borrowed type recipe callbacks, initialize constants, then construct exactly one explicit root. Root output starts empty; caller uses stop to clean completed prefixes after failure. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_start(AmaProgram *, uint64_t, AmaHandle *);
/** Destroy a present root, reverse-clean constants and dispose finalized pools. Preserve the initial failure and mark failed execution terminal before cleanup. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_stop(AmaProgram *, AmaHandle, AmaStatus);
/** Resolve one target and return explicit acquisition-refusal provenance separately from status; protocol errors cannot be classified by numeric coincidence. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05), [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [FLOW-05](../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_select_target(AmaProgram *, AmaHandle, const AmaProgramAccess *, AmaProgramValue *, bool *);
#endif
