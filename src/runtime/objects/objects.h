#ifndef AMA_RUNTIME_OBJECTS_H
#define AMA_RUNTIME_OBJECTS_H

#include "runtime/pool/pool.h"
#include "native/abi/bytes.h"
#include "source/memory/arena.h"

typedef enum { AMA_FIELD_OWNED, AMA_FIELD_REFERENCE, AMA_FIELD_OWNING_LIST, AMA_FIELD_REFERENCE_LIST } AmaFieldKind;
typedef struct AmaRecipe AmaRecipe;
typedef struct {
    size_t count;
    const AmaRecipe *children;
    const AmaHandle *references;
} AmaFieldRecipe;
struct AmaRecipe {
    AmaConstructorKind constructor;
    AmaByteView literal;
    AmaHandle source;
    bool continuation;
    const AmaFieldRecipe *fields;
    size_t field_count;
};
typedef struct { AmaHandle *items; size_t count; } AmaHandleList;
typedef struct { AmaFieldKind kind; uint64_t type_id; size_t offset; } AmaObjectField;
typedef struct {
    AmaStatus (*initialize)(void *);
    AmaStatus (*literal)(void *, AmaByteView);
    AmaStatus (*clone)(void *, const void *);
    AmaStatus (*destroy)(void *);
} AmaPayloadHooks;
typedef struct {
    AmaPool pool;
    const AmaObjectField *fields;
    size_t field_count;
    const AmaFieldRecipe *defaults;
    size_t native_offset;
    AmaPayloadHooks hooks;
} AmaObjectType;
typedef struct {
    AmaObjectType *const *types;
    size_t type_count;
    SourceAllocator allocator;
    void *report_context;
    void (*report)(void *, AmaStatus);
    bool operation_active;
    bool failed;
} AmaWorld;

/* Generated descriptors/recipes and the registry are borrowed. Persistent Ama
 * edges occupy declared handle fields only. Native hooks operate on payloads,
 * cannot reenter Ama, and must clean their own failed constructor resources.
 * These helpers execute lifecycle operations, never module bodies. */
AmaStatus ama_object_type_init(AmaObjectType *, SourceAllocator, uint64_t,
    size_t size, size_t slots, uint64_t generation_limit, const AmaObjectField *,
    size_t field_count, size_t native_offset, AmaPayloadHooks);
AmaObjectType *ama_world_type(const AmaWorld *, uint64_t);
AmaStatus ama_object_create(AmaWorld *, uint64_t, const AmaRecipe *, AmaHandle *);
AmaStatus ama_object_destroy(AmaWorld *, AmaHandle);
AmaStatus ama_object_clone(AmaWorld *, AmaHandle source, bool continuation, AmaHandle *);
AmaStatus ama_object_replace(AmaWorld *, AmaHandle destination, AmaHandle source, bool continuation);

#endif
