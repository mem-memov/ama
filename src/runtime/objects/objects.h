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
 */
#ifndef AMA_RUNTIME_OBJECTS_H
#define AMA_RUNTIME_OBJECTS_H

#include "runtime/pool/pool.h"
#include "runtime/lists/membership/membership.h"
#include "native/abi/bytes.h"
#include "source/memory/arena.h"

/** Storage form separates ownership from scalar/list shape; references never acquire lifecycle ownership. */
typedef enum { AMA_FIELD_OWNED, AMA_FIELD_REFERENCE, AMA_FIELD_OWNING_LIST, AMA_FIELD_REFERENCE_LIST } AmaFieldKind;
/**
 * Forward name for borrowed selected constructor recipes; the complete fields and
 * source-lifetime contract follow.
 */
typedef struct AmaRecipe AmaRecipe;
/** Borrowed selected field initialization recipe; supplied distinguishes an explicit empty list from absent defaults, operation locates failures. */
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
 */
struct AmaRecipe {
    AmaConstructorKind constructor;
    AmaByteView literal;
    AmaHandle source;
    bool continuation;
    const void *native_source;
    AmaStatus (*from_size)(void *, size_t);
    size_t size_value;
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
 */
AmaHandle *ama_handle_list_element(const AmaHandleList *, size_t);
/** Prepare a new private list shape at its final exact namespace/incarnation.
 * Own partial metadata on failure; no target constructors or bodies run.
 */
AmaStatus ama_handle_list_prepare(AmaHandleList *, SourceAllocator, AmaHandle owner,
    size_t field, uint64_t incarnation, uint64_t counter_limit, size_t count);
/** Validated nominal field descriptor with exact C payload offset; C layout truth is a generated-descriptor precondition. */
typedef struct { AmaFieldKind kind; uint64_t type_id; size_t offset; } AmaObjectField;
/** Synchronous native lifecycle hooks; successful constructors create one destroy credit, failed constructors clean their own partial resources and cannot reenter Ama. */
typedef struct {
    AmaStatus (*initialize)(void *);
    AmaStatus (*literal)(void *, AmaByteView);
    AmaStatus (*clone)(void *, const void *);
    AmaStatus (*destroy)(void *);
} AmaPayloadHooks;
/**
 * Forward name for the borrowed registry, allocator and synchronous lifecycle operation
 * authority; complete ownership is defined below.
 */
typedef struct AmaWorld AmaWorld;
/** One nominal pool plus borrowed field/default/hook descriptors and recipe callback context. Registry/type storage must outlive all instances. */
typedef struct {
    AmaPool pool;
    const AmaObjectField *fields;
    size_t field_count;
    const AmaFieldRecipe *defaults;
    size_t native_offset;
    AmaPayloadHooks hooks;
    uint64_t source_operation;
    AmaStatus (*resolve_field)(AmaWorld *, SourceArena *, AmaHandle, size_t, AmaFieldRecipe *, void *);
    void *field_context;
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
/** Initialize the validated object layout with default max_align_t payload alignment; explicit extended alignment uses the aligned interface. */
AmaStatus ama_object_type_init(AmaObjectType *, SourceAllocator, uint64_t,
    size_t size, size_t slots, uint64_t generation_limit, const AmaObjectField *,
    size_t field_count, size_t native_offset, AmaPayloadHooks);
/** Validate truthful generated field extents/alignment/disjointness and compatible hook presence, then initialize an empty aligned pool. Descriptors remain borrowed; no native constructor runs. */
AmaStatus ama_object_type_init_aligned(AmaObjectType *, SourceAllocator, uint64_t,
    size_t size, size_t alignment, size_t slots, uint64_t generation_limit,
    const AmaObjectField *, size_t field_count, size_t native_offset, AmaPayloadHooks);
/** Resolve exact nominal type ID in a unique borrowed registry, or null; no allocation or source-name matching. */
AmaObjectType *ama_world_type(const AmaWorld *, uint64_t);
/** Own guarded private construction and publish output only after complete success. Failure unwinds the initialized prefix; native module bodies are not executed. */
AmaStatus ama_object_create(AmaWorld *, uint64_t, const AmaRecipe *, AmaHandle *);
/** Acquire one world operation guard, preflight normal destruction or clear terminal active links, then clean the tree. Always release protections and scratch, preserving the primary failure. */
AmaStatus ama_object_destroy(AmaWorld *, AmaHandle);
/** Own guarded whole-tree copy and exact reference fixups, then publish fresh identities. Ordinary mode clears positions, explicit continuation mode retains eligible saved positions. */
AmaStatus ama_object_clone(AmaWorld *, AmaHandle source, bool continuation, AmaHandle *);
/** Own one nonreentrant world operation, its protection records and scratch for replacement; release all temporary authority on every return. */
AmaStatus ama_object_replace(AmaWorld *, AmaHandle destination, AmaHandle source, bool continuation);

#endif
