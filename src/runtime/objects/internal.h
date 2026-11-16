/**
 * @file
 * @brief Declare private construction, protection, copy and cleanup phase ownership.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * [LIFE-04](../../../spec/implementation/native-interface.md#life-04) separates
 * private preparation handles from the retained destination identity before commit.
 * Operation arenas own scratch; work and protection records borrow stable slots. Callers must release protections before destroying scratch.
 * [CONT-04](../../../spec/language/continuations.md#cont-04) validates and maps retained compiler control cells
 * separately from native payload hooks; ordinary copies leave them inactive.
 */
#ifndef AMA_RUNTIME_OBJECTS_INTERNAL_H
#define AMA_RUNTIME_OBJECTS_INTERNAL_H
#include "objects.h"
#include "runtime/lifecycle/errors.h"

/** Arena-owned private construction/clone cursor; registered stable cells own payloads, clone_root scopes exact remapping boundaries and parent tracks initialized-prefix unwind. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct AmaObjectWork {
    struct AmaObjectWork *parent;
    struct AmaObjectWork *next;
    struct AmaObjectWork *clone_root;
    AmaObjectType *type;
    AmaPoolCell *cell;
    AmaHandle handle;
    /** Final language-visible self; replacement roots retain the destination ID,
     * while handle identifies private construction storage until commit.
     * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
    AmaHandle identity;
    AmaHandle source;
    const AmaRecipe *recipe;
    /** Selected clone mode survives the caller's temporary recipe object. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
    bool continuation;
    uint64_t operation;
    size_t next_child;
    size_t next_field;
    size_t next_element;
    bool field_ready;
    AmaFieldRecipe field_recipe;
} AmaObjectWork;
/** Operation-owned read/destruction protection record; previous captures service state, epoch detects duplicate ownership and restore identifies sole restoration authority. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct AmaObjectProtection {
    struct AmaObjectProtection *next;
    AmaObjectType *type;
    AmaPoolCell *cell;
    AmaHandle handle;
    AmaServiceState previous;
    size_t epoch;
    bool copy;
    bool restore;
    /** Operation-local stackless validation state; never mutates source data. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
    struct AmaObjectProtection *control_parent;
    size_t control_child;
    bool controls_validated;
} AmaObjectProtection;
/** One guarded synchronous lifecycle operation. Own arena/protection/work lists; borrow source_authority only while its outer operation remains alive. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
typedef struct {
    AmaWorld *world;
    SourceArena arena;
    AmaObjectWork *first;
    AmaObjectWork *last;
    AmaObjectProtection *protected_first;
    AmaObjectProtection *protected_last;
    size_t protection_epoch;
    const AmaObjectProtection *source_authority;
    void *root_payload;
    /** Existing protected destination identity for a noncopy replacement root;
     * empty means a fresh root whose reserved handle is already final.
     * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
    AmaHandle root_identity;
} AmaObjectOperation;

/** Validate empty/type/slot identity before exposing a cell. Private lookup is for trusted construction phases; type output may be selected before a later slot failure. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_lookup(AmaWorld *, AmaHandle, bool private_slot, AmaObjectType **, AmaPoolCell **);
/** Synchronously report a non-success status through the optional borrowed callback; the callback cannot reenter Ama. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
void ama_object_report(AmaWorld *, AmaStatus);
/** Compare complete local handle triples without implying availability or liveness. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
bool ama_same_handle(AmaHandle, AmaHandle);
/** Read a validated scalar/list field shape; no target is acquired. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
size_t ama_object_field_count(const AmaObjectType *, void *payload, size_t field);
/** Borrow one validated field element; list records stay stable across growth. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaHandle *ama_object_field_handle(const AmaObjectType *, void *payload, size_t field, size_t index);
/** Prepare an empty/private list at the final owner namespace, advancing the
 * old field incarnation for root replacement before any destructive commit.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_list_prepare(AmaWorld *, AmaObjectWork *, size_t field, size_t count);
/** Recognize only declared scalar/list ownership forms; references never imply lifecycle ownership. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
bool ama_field_owns(AmaFieldKind);
/** Recognize the two list storage forms independently of whether they own their targets. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
bool ama_field_list(AmaFieldKind);
/** Invoke the selected native clone hook into private stable storage and credit native ownership only after success; source remains protected. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_clone_payload(AmaObjectOperation *, AmaObjectWork *, bool);
/** Rewrite internal declared references through the exact operation copy map and final-root identity; preserve external targets without acquiring them. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_fix_references(AmaObjectOperation *, AmaObjectWork *, AmaHandle, bool copy_continuation);
/** Validate compiler continuation controls across an already protected complete
 * owned tree. Reuse operation records as a stackless traversal/cache; allocate
 * no snapshots or additional native/Ama objects.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_control_preflight(AmaObjectOperation *, AmaHandle);
/** Prepare private field storage and child counts from validated source descriptors; partial buffers remain registered for operation cleanup. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_clone_fields(AmaWorld *, AmaObjectWork *);
/** Resolve normal public storage first; only a stale public lookup may match exact completed private work owned by this operation. Never expose unrelated reserved storage. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_source_lookup(AmaObjectOperation *, AmaHandle, AmaObjectType **, AmaPoolCell **);
/** Validate supplied recipe field count and begin the private lifecycle ledger; construction phases populate and credit the exact owned-child prefix. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_fields(AmaWorld *, AmaObjectWork *);
/** Resolve one valid flattened owned-child index to its declared handle location/type. Reference fields are skipped; the caller supplies an initialized-prefix index. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaHandle *ama_object_child(AmaObjectWork *, size_t index, uint64_t *type_id, const AmaRecipe **);
/** Validate destination availability and both identities, then exact runtime self-copy and overlap. Distinct-source availability is checked by whole-tree preflight before cloning. Ordinary exact self is effect-free; continuation self and ancestor/descendant overlap fail. Static clone capability remains mandatory in the compiler. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_copy_checks(AmaWorld *, AmaHandle, AmaHandle, bool, bool *);
/** Walk the initialized owned tree using owner links and cell cursors, clearing transient service controls without allocation or recursive C calls. This is terminal cleanup only. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_end_active_links(AmaWorld *, AmaHandle);
/** Iteratively invalidate and finalize one initialized tree, native resource first and children in reverse. Continue after destructor errors, marking the world terminal; optionally retain the root slot for replacement. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_cleanup_keep(AmaWorld *, AmaHandle root, AmaStatus primary, bool keep_root);
/** Clean the complete initialized tree and release its root slot, retaining the supplied primary failure over secondary cleanup errors. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_cleanup(AmaWorld *, AmaHandle root, AmaStatus primary);
/** Append a completely published independent root to successful-creation order.
 * The stable cell is unlinked and exclusively owned by this world; no allocation or hook runs.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
void ama_object_root_append(AmaWorld *, AmaHandle, AmaPoolCell *);
/** Unlink a registered root before slot release; private work and owned children are no-ops.
 * Keeping a root slot for replacement must preserve its links and original position.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
void ama_object_root_remove(AmaWorld *, AmaPoolCell *);
/** Compute finite monotone reachability over every declared owned field, including empty-list element types. Reject any reachable native resource lacking clone; scratch remains operation-owned. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_copy_capability(AmaObjectOperation *, uint64_t);
/** Protect the complete initialized owned tree before hooks or destruction. Follow ownership only, check declared copy capability and nonwrapping epochs; caller unprotects partial records on failure. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_preflight(AmaObjectOperation *, AmaHandle, bool copy, bool continuation);
/** Restore only records that own restoration authority and still identify the same occupied generation; never resurrect released or replaced identities. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
void ama_object_unprotect(AmaObjectOperation *);
/** Iteratively construct private work and publish only after all phases succeed; failures unwind exactly the initialized prefix without recursive C calls. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_build(AmaObjectOperation *, uint64_t, const AmaRecipe *, AmaHandle *);
/** Build a protected private clone under one copy boundary and fix internal references before publication. Final-root redirection preserves replacement identity. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_clone_prepare(AmaObjectOperation *, AmaHandle, AmaHandle final_root,
    bool continuation, void *root_payload, AmaHandle *);
/** After successful old-value cleanup, flip to the prepared payload bank and publish the new ledger/children under the unchanged root identity. Release only scratch root identity; native payloads are never moved. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
void ama_object_replacement_publish(AmaObjectOperation *, AmaObjectType *, AmaPoolCell *, AmaHandle);
/** Apply exact copy/overlap checks inside a caller-owned guarded operation; prepare/commit only when runtime identity is not an ordinary self-copy. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_replace_in(AmaObjectOperation *, AmaHandle, AmaHandle, bool);
/** Within an existing operation, select direct clone replacement or one default/literal recipe in the inactive bank. No default-then-overwrite construction and no payload relocation. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_reconstruct_in(AmaObjectOperation *, AmaHandle, const AmaRecipe *);
/** Check exact root equality or validated owner ancestry using the finite occupied-population bound; output is meaningful only on success. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03), [LIFE-04](../../../spec/implementation/native-interface.md#life-04), [CONT-04](../../../spec/language/continuations.md#cont-04). */
AmaStatus ama_object_contains(AmaWorld *, AmaHandle, AmaHandle, bool *);
#endif
