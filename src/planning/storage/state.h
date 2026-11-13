/** @file
 * @brief Describe context-local domains and fixed per-instance lexical control storage.
 * [RT-02](../../../spec/implementation/runtime.md#rt-02) places cursors/cells in
 * their owning instance; [GEN-03](../../../spec/implementation/compiler.md#gen-03)
 * keeps transfer-surviving values out of C automatic activations.
 */
#ifndef AMA_PLANNED_STORAGE_STATE_H
#define AMA_PLANNED_STORAGE_STATE_H
#include "semantic/semantic.h"
/** Logical ownership/cardinality, independent of physical C representation. */
typedef enum { STORAGE_OWNED, STORAGE_REFERENCE, STORAGE_OWNING_LIST, STORAGE_REFERENCE_LIST } StorageForm;
/** Per-context runtime domains, instantiated independently even when code/descriptors are shared. */
enum {
    CONTEXT_POOL_DIRECTORY = 1u, CONTEXT_SHARED_VALUES = 2u, CONTEXT_CURRENT_INSTANCE = 4u,
    CONTEXT_EXECUTOR = 8u, CONTEXT_ROOT_LEDGER = 16u, CONTEXT_DIAGNOSTICS = 32u
};
/** Exact counts accompany the general context representation; no program-wide mutable payload mirror exists. */
typedef struct {
    unsigned domains;
    size_t pools;
    size_t constants;
} ContextStorage;
/** Different lifetime domains must not be confused by continuation cloning. */
typedef enum { STORAGE_OPERATION_FRAME, STORAGE_TRAVERSAL_FRAME } StorageFrameKind;
/** Explicit logical frame contents; flags identify information, not mandatory C fields or byte offsets. */
enum {
    FRAME_TARGET_PROTECTION = 1u, FRAME_BODY_RETURN = 2u, FRAME_OUTPUT_CONTINUATION = 4u,
    FRAME_INPUT_PROTECTION = 8u, FRAME_INTRINSIC_RESULT = 16u,
    FRAME_CURSOR_ACTIVITY = 32u, FRAME_COLLECTION_IDENTITY = 64u, FRAME_BOUNDARY = 128u,
    FRAME_CURRENT_MEMBERSHIP = 256u, FRAME_BINDING_ORIGIN = 512u
};
/** Forward identity for a fixed lexical site, never a separately allocated runtime activation. */
typedef struct StorageFrame StorageFrame;
/** One conditional scalar reference cell; an empty traversal never activates it or owns its referent. */
typedef struct ScopedStorageCell {
    struct ScopedStorageCell *next;
    const SemanticSymbol *symbol;
    const SemanticForeachBinding *introduction;
    size_t slot;
} ScopedStorageCell;
/** One origin/active-role association per reused reference cell, separate from its stored handle.
 * The handle remains in its permanent field or scoped cell; nested roles share
 * this record and do not save/restore or duplicate that handle.
 */
typedef struct ReferenceOriginStorage {
    struct ReferenceOriginStorage *next;
    const SemanticSymbol *symbol;
    size_t slot;
} ReferenceOriginStorage;
/** One fixed frame slot per lexical operation/traversal in each independent instance.
 * Operation frames are transient and never copied as pending returns. Traversal
 * frames/cells are resumable control state under CONT-04. Parents name separate
 * operation and traversal ancestry; no runtime call-history lookup is needed.
 */
struct StorageFrame {
    StorageFrame *next;
    StorageFrame *operation_parent;
    StorageFrame *traversal_parent;
    const SemanticStatement *statement;
    const ScopedStorageCell *cell;
    const ReferenceOriginStorage *origin;
    const SemanticSymbol *persistent_binding;
    StorageFrameKind kind;
    unsigned contents;
    size_t slot;
};
#endif
