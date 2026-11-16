/**
 * @file
 * @brief Expose synchronous ownership and lifecycle operations over stable typed pools.
 *
 * [RT-02](../../../spec/implementation/runtime.md#rt-02)
 * Borrow immutable descriptors and recipes. Native hooks cannot reenter Ama and must clean resources of a failed constructor themselves.
 * [LIFE-04](../../../spec/implementation/native-interface.md#life-04) keeps
 * final replacement identity separate from temporary construction storage;
 * the world borrows construction authority only during a field resolver.
 * [LIFE-05](../../../spec/implementation/native-interface.md#life-05) requires
 * independent roots to retain successful-publication order through replacement.
 * [PROTO-03](../../../spec/implementation/native-interface.md#proto-03) supplies
 * the fixed integer host-size constructor; its internal recipe is selected by
 * exact compiler identity, never exposed as an Ama machine-value source or cast.
 * [LIST-13](../../../spec/language/lists.md#list-13) supplies immutable lexical
 * cursor catalogs so mutations correct only matching active collection identities.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) selects one
 * compiler-owned transport constructor in fresh native storage; its C input
 * borrow ends before body execution and never enters persistent state.
 * [CONT-04](../../../spec/language/continuations.md#cont-04) validates and maps retained compiler control cells
 * separately from native payload hooks; ordinary copies leave them inactive.
 */
#ifndef AMA_RUNTIME_OBJECTS_H
#define AMA_RUNTIME_OBJECTS_H

#include "runtime/pool/pool.h"
#include "runtime/lists/membership/membership.h"
#include "runtime/lists/cursors/model.h"
#include "native/abi/bytes.h"
#include "source/memory/arena.h"

/** Storage form separates ownership from scalar/list shape; references never acquire lifecycle ownership. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef enum { AMA_FIELD_OWNED, AMA_FIELD_REFERENCE, AMA_FIELD_OWNING_LIST, AMA_FIELD_REFERENCE_LIST } AmaFieldKind;
/**
 * Forward name for borrowed selected constructor recipes; the complete fields and
 * source-lifetime contract follow.
 * Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct AmaRecipe AmaRecipe;
/** Borrowed selected field initialization recipe; supplied distinguishes an explicit empty list from absent defaults, operation locates failures. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct {
    size_t count;
    const AmaRecipe *children;
    const AmaHandle *references;
    bool supplied;
    uint64_t operation;
} AmaFieldRecipe;
/** One selected constructor plus borrowed literal/source/field data. Source lifetime
 * and operation protections cover construction. from_size is a trusted compiler's
 * exact typed bridge adapter for the fixed standard-library integer; size_value
 * is internal service metadata, not an Ama source form or generic conversion.
 * Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
struct AmaRecipe {
    AmaConstructorKind constructor;
    AmaByteView literal;
    AmaHandle source;
    bool continuation;
    const void *native_source;
    AmaStatus (*from_size)(void *, size_t);
    size_t size_value;
    /** Compiler-owned decoder of one declared schema, borrowing C input only during construction.
     * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) constructs
     * fresh typed payloads without first running the default constructor.
     */
    AmaStatus (*from_transport)(void *, const void *);
    const void *transport_input;
    const AmaFieldRecipe *fields;
    size_t field_count;
};
/** Declared list storage uses exact occurrence records under
 * [LIST-03](../../../spec/language/lists.md#list-03); targets remain stable pool handles.
 */
typedef AmaMembershipList AmaHandleList;
/** Borrow one current protected target cell by validated position. Private
 * construction/fixup may populate it; public reference replacement must retire
 * the occurrence through ama_membership_replace rather than writing this cell.
 * Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaHandle *ama_handle_list_element(const AmaHandleList *, size_t);
/** Prepare a new private list shape at its final exact namespace/incarnation.
 * Own partial metadata on failure; no target constructors or bodies run.
 * Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_handle_list_prepare(AmaHandleList *, SourceAllocator, AmaHandle owner,
    size_t field, uint64_t incarnation, uint64_t counter_limit, size_t count);
/** Validated nominal field descriptor with exact C payload offset; C layout truth is a generated-descriptor precondition. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct { AmaFieldKind kind; uint64_t type_id; size_t offset; } AmaObjectField;
/** Synchronous native lifecycle hooks; successful constructors create one destroy credit, failed constructors clean their own partial resources and cannot reenter Ama. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct {
    AmaStatus (*initialize)(void *);
    AmaStatus (*literal)(void *, AmaByteView);
    AmaStatus (*clone)(void *, const void *);
    AmaStatus (*destroy)(void *);
} AmaPayloadHooks;
/**
 * Forward name for the borrowed registry, allocator and synchronous lifecycle operation
 * authority; complete ownership is defined below.
 * Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct AmaWorld AmaWorld;
/** Exact selected ownership-boundary map; external/empty handles are preserved.
 * The borrowed context is valid only during a synchronous compiler callback.
 * Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef AmaHandle (*AmaObjectReferenceMap)(const void *, AmaHandle);
/** Borrow complete protected source and private destination control storage only
 * after their declared fields/native payloads are ready. No pointer is retained.
 * Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct {
    AmaWorld *world;
    AmaHandle source, destination;
    const void *source_storage;
    void *destination_storage;
    AmaObjectReferenceMap map;
    const void *map_context;
} AmaObjectControlCopy;
/** Compiler-owned control metadata phases, not native resource/ABI protocols.
 * Validation precedes continuation preparation; copy/map never execute Ama,
 * acquire targets or retain borrowed map/storage pointers.
 * Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct {
    AmaStatus (*validate_continuation)(AmaWorld *, AmaHandle, const void *, const AmaServiceState *, void *);
    AmaStatus (*copy_continuation)(const AmaObjectControlCopy *, void *);
    AmaStatus (*map_references)(AmaWorld *, uint64_t, void *, AmaObjectReferenceMap, const void *, void *);
} AmaObjectControlHooks;
/** One nominal pool plus borrowed field/default/hook descriptors and recipe callback context. Registry/type storage must outlive all instances. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct {
    AmaPool pool;
    const AmaObjectField *fields;
    size_t field_count;
    /** Immutable fixed lexical cursor catalog, indexed by declared field. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
    const AmaListCursorSet *list_cursors;
    const AmaFieldRecipe *defaults;
    size_t native_offset;
    AmaPayloadHooks hooks;
    uint64_t source_operation;
    AmaStatus (*resolve_field)(AmaWorld *, SourceArena *, AmaHandle, size_t, AmaFieldRecipe *, void *);
    void *field_context;
    const AmaObjectControlHooks *controls;
    void *control_context;
} AmaObjectType;
/** Borrowed unique type registry, allocator and synchronous report authority;
 * operation_active rejects reentry, failed marks terminal lifecycle failure.
 * first_root/last_root borrow stable cells in successful publication order under
 * [LIFE-05](../../../spec/implementation/native-interface.md#life-05); no payload
 * ownership is transferred by these runtime-only links. Not a shared-thread context API.
 */
struct AmaWorld {
    AmaObjectType *const *types;
    size_t type_count;
    SourceAllocator allocator;
    void *report_context;
    void (*report)(void *, AmaStatus);
    uint64_t construction_operation;
    /** Borrow the exact work record only during its synchronous field resolver;
     * [LIFE-04](../../../spec/implementation/native-interface.md#life-04) keeps
     * replacement storage separate from its final language-visible identity.
     * No callback may retain this pointer or reenter field resolution.
     */
    const struct AmaObjectWork *construction_work;
    bool operation_active;
    bool failed;
    AmaPoolCell *first_root;
    AmaPoolCell *last_root;
};

/* Generated descriptors/recipes and the registry are borrowed. Persistent Ama
 * edges occupy declared handle fields only. Native hooks operate on payloads,
 * cannot reenter Ama, and must clean their own failed constructor resources.
 * These helpers execute lifecycle operations, never module bodies. */
/** Initialize the validated object layout with default max_align_t payload alignment; explicit extended alignment uses the aligned interface. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_type_init(AmaObjectType *, SourceAllocator, uint64_t,
    size_t size, size_t slots, uint64_t generation_limit, const AmaObjectField *,
    size_t field_count, size_t native_offset, AmaPayloadHooks);
/** Validate truthful generated field extents/alignment/disjointness and compatible hook presence, then initialize an empty aligned pool. Descriptors remain borrowed; no native constructor runs. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_type_init_aligned(AmaObjectType *, SourceAllocator, uint64_t,
    size_t size, size_t alignment, size_t slots, uint64_t generation_limit,
    const AmaObjectField *, size_t field_count, size_t native_offset, AmaPayloadHooks);
/** Resolve exact nominal type ID in a unique borrowed registry, or null; no allocation or source-name matching. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaObjectType *ama_world_type(const AmaWorld *, uint64_t);
/** Own guarded private construction and publish output only after complete success. Failure unwinds the initialized prefix; native module bodies are not executed. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_create(AmaWorld *, uint64_t, const AmaRecipe *, AmaHandle *);
/** Return explicit destructive-admission provenance independently of native hook status.
 * *admitted starts false and becomes true only after successful access preflight.
 * Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_destroy_admitted(AmaWorld *, AmaHandle, bool *admitted);
/** Acquire one world operation guard, preflight normal destruction or clear terminal active links, then clean the tree. Always release protections and scratch, preserving the primary failure. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_destroy(AmaWorld *, AmaHandle);
/** Own guarded whole-tree copy and exact reference fixups, then publish fresh identities. Ordinary mode clears positions, explicit continuation mode retains eligible saved positions. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_clone(AmaWorld *, AmaHandle source, bool continuation, AmaHandle *);
/** Own one nonreentrant world operation, its protection records and scratch for replacement; release all temporary authority on every return. Component contract: [RT-02](../../../spec/implementation/runtime.md#rt-02), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [LIFE-05](../../../spec/implementation/native-interface.md#life-05), [PROTO-03](../../../spec/implementation/native-interface.md#proto-03), [LIST-13](../../../spec/language/lists.md#list-13), [EMBED-04](../../../spec/implementation/c-library.md#embed-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_replace(AmaWorld *, AmaHandle destination, AmaHandle source, bool continuation);

#endif
