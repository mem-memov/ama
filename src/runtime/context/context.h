/**
 * @file
 * @brief Own independent runtime contexts behind checked host identities and linear access leases.
 *
 * [PAR-01](../../../spec/language/async-and-parallelism.md#par-01) isolates mutable Ama state.
 * [REF-02](../../../spec/language/values-and-references.md#ref-02) requires context validation
 * before a host token can select a local handle. This internal foundation is not an export ABI.
 * [DEBUG-02](../../../spec/implementation/diagnostics.md#debug-02) carries an explicit
 * initialization operation for allocation-free diagnostics before context storage exists.
 * [LIST-13](../../../spec/language/lists.md#list-13) shares only immutable lexical
 * cursor catalogs; each context owns its actual active/suspended cursor slots.
 * [FLOW-05](../../../spec/language/lists.md#flow-05) Immutable scope descriptors preserve exact lexical ancestry while each context owns its binding cells.
 */
#ifndef AMA_RUNTIME_CONTEXT_H
#define AMA_RUNTIME_CONTEXT_H

#include <stdatomic.h>
#include "runtime/metadata/runtime.h"
#include "runtime/program/program.h"
#include "runtime/program/diagnostics/trace.h"

/** Immutable, compiler-resolved executable type schema. Arrays, hooks and source views
 * are borrowed through every context built from it; outer records are read during creation.
 * No pool or native State is shared.
 */
typedef struct {
    uint64_t type_id;
    size_t size;
    size_t alignment;
    size_t slots_per_block;
    uint64_t generation_limit;
    const AmaObjectField *fields;
    const AmaProgramField *declarations;
    size_t field_count;
    size_t native_offset;
    AmaPayloadHooks hooks;
    uint64_t operation;
    AmaStatus (*condition)(const void *, bool *);
    AmaStatus (*index)(const void *, size_t *);
    /** Borrowed lexical cursor catalog, with one entry per declared field. */
    const AmaListCursorSet *list_cursors;
    /** Borrowed exact scope ancestry and binding locations, checked before creation. */
    const struct AmaProgramForeachScope *foreach_scopes;
    size_t foreach_count;
} AmaContextType;

/** Immutable shared declaration; each context allocates its own storage and lifecycle ledger. */
typedef struct {
    uint64_t type_id;
    bool list;
    const AmaProgramSource *initializers;
    size_t initializer_count;
    uint64_t operation;
} AmaContextConstant;

/** Borrowed resolved schema and synchronous reporting callback. The allocator and callback
 * must support the host's chosen concurrency; callbacks cannot reenter the active context.
 * Native-resource affinity must permit any host-thread transfer; a gate does not grant affinity.
 */
typedef struct {
    const AmaContextType *types;
    size_t type_count;
    const AmaContextConstant *constants;
    size_t constant_count;
    const AmaProgramLocation *locations;
    size_t location_count;
    /** Exact generated source operation for failure before a root or descriptor arena exists. */
    uint64_t initialization_operation;
    void (*report)(void *, AmaStatus);
    void *report_context;
} AmaContextDefinition;

/** Opaque registry-owned context. A host token never contains its C address. */
typedef struct AmaContext AmaContext;

/** Explicit host-owned identity registry. Initialize once per C object lifetime and keep
 * it alive through every operation/lease. Its atomic gate protects membership only, not Ama bodies.
 * Closing contexts leaves this owner valid; destroy requires exclusive C lifetime authority.
 */
typedef struct {
    _Atomic bool locked;
    SourceAllocator allocator;
    AmaContext *first;
    size_t pending;
    bool initialized;
    bool closed;
    AmaStatus close_status;
} AmaContextRegistry;

/** Checked host instance identity. Context zero is empty; the local triple has meaning
 * only after the originating context has been validated. This is not a transport value.
 */
typedef struct {
    uint64_t context;
    AmaHandle local;
} AmaContextReference;

/** Linear internal access owner, initialized empty and never copied. A live lease pins its
 * record and owns its context gate. Borrowed views expire before release; forged or shallowly
 * copied lease pointers violate the C ownership contract, unlike copyable numeric host tokens.
 */
typedef struct {
    AmaContextRegistry *registry;
    AmaContext *context;
    uint64_t identity;
} AmaContextLease;

/** Operation-scoped internal runtime authority. These pointers never cross the public
 * compiled-component boundary and must not survive the supplying lease.
 */
typedef struct {
    AmaWorld *world;
    AmaProgram *program;
    AmaRuntimeMetadata *control;
    AmaStatus status;
} AmaContextView;

/** Initialize a fresh registry with borrowed malloc/free-compatible allocation authority.
 * Failure leaves an uninitialized, safely destroyable owner. No Ama constructor runs.
 */
AmaStatus ama_context_registry_init(AmaContextRegistry *registry, SourceAllocator allocator);
/** Close every context after acquiring all their gates; busy/pending work leaves membership
 * unchanged. Accepted closure invalidates identities before hooks, cleans all roots/constants
 * and retains the first failure. Repeated completed closure returns that same outcome.
 */
AmaStatus ama_context_registry_close(AmaContextRegistry *registry);
/** End a registry's C lifetime after successful admission to close. A busy owner remains
 * initialized; accepted cleanup errors do not prevent ending this owner exactly once.
 */
AmaStatus ama_context_registry_destroy(AmaContextRegistry *registry);
/** Construct context-local descriptors and shared constants without an implicit root/body.
 * Output must start zero and changes only on complete publication. Failure cleans every prefix;
 * immutable resolved schemas must satisfy ordinary compiler descriptor/initializer contracts.
 */
AmaStatus ama_context_create(AmaContextRegistry *registry, const AmaContextDefinition *definition,
    uint64_t *identity);
/** Invalidate one checked context before cleaning its resources. A held lease refuses busy
 * without effects; stale identities are rejected without dereferencing freed context storage.
 */
AmaStatus ama_context_dispose(AmaContextRegistry *registry, uint64_t identity);
/** Acquire one nonreentrant context gate into an empty linear lease. Registry membership is
 * validated before touching context state. Different contexts can hold leases simultaneously.
 */
AmaStatus ama_context_enter(AmaContextRegistry *registry, uint64_t identity, AmaContextLease *lease);
/** Consume one valid lease and publish all prior context writes before another executor enters.
 * An empty lease is invalid; no context pointer may be accessed after releasing its gate.
 */
AmaStatus ama_context_leave(AmaContextLease *lease);
/** Borrow this lease's runtime owners and current first failure. Failed contexts permit this
 * diagnostic inspection but ordinary reference resolution is rejected. After
 * execution cleanup, descriptor arrays/counts are empty and only status/control
 * inspection remains; every supplied view expires when its lease is released.
 */
AmaStatus ama_context_inspect(const AmaContextLease *lease, AmaContextView *view);
/** Wrap an internal context-local published identity; do not acquire its target or run code.
 * The local input must originate in this lease, not unchecked foreign host data.
 */
AmaStatus ama_context_reference(const AmaContextLease *lease, AmaHandle local,
    AmaContextReference *reference);
/** Check origin before local identity/access checks. Output pointers borrow the active lease
 * only on success; technical failure and wrong-context rejection do not mutate Ama state.
 */
AmaStatus ama_context_resolve(const AmaContextLease *lease, AmaContextReference reference,
    AmaAccessMode mode, AmaObjectType **type, AmaPoolCell **cell);

#endif
