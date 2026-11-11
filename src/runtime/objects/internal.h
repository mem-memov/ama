#ifndef AMA_RUNTIME_OBJECTS_INTERNAL_H
#define AMA_RUNTIME_OBJECTS_INTERNAL_H
#include "objects.h"
#include "runtime/lifecycle/errors.h"

typedef struct AmaObjectWork {
    struct AmaObjectWork *parent;
    struct AmaObjectWork *next;
    struct AmaObjectWork *clone_root;
    AmaObjectType *type;
    AmaPoolCell *cell;
    AmaHandle handle;
    AmaHandle source;
    const AmaRecipe *recipe;
    size_t next_child;
} AmaObjectWork;
typedef struct AmaObjectProtection {
    struct AmaObjectProtection *next;
    AmaObjectType *type;
    AmaPoolCell *cell;
    AmaHandle handle;
    AmaServiceState previous;
    bool copy;
    bool restore;
} AmaObjectProtection;
typedef struct {
    AmaWorld *world;
    SourceArena arena;
    AmaObjectWork *first;
    AmaObjectWork *last;
    AmaObjectProtection *protected_first;
    AmaObjectProtection *protected_last;
} AmaObjectOperation;

AmaStatus ama_object_lookup(AmaWorld *, AmaHandle, bool private_slot, AmaObjectType **, AmaPoolCell **);
void ama_object_report(AmaWorld *, AmaStatus);
bool ama_same_handle(AmaHandle, AmaHandle);
AmaHandle *ama_object_handles(const AmaObjectType *, void *payload, size_t field, size_t *count);
bool ama_field_owns(AmaFieldKind);
bool ama_field_list(AmaFieldKind);
AmaStatus ama_object_clone_payload(AmaObjectOperation *, AmaObjectWork *, bool);
void ama_object_fix_references(AmaObjectOperation *, AmaObjectWork *, AmaHandle);
AmaStatus ama_object_clone_fields(AmaWorld *, AmaObjectWork *);
AmaStatus ama_object_fields(AmaWorld *, AmaObjectWork *);
AmaHandle *ama_object_child(AmaObjectWork *, size_t index, uint64_t *type_id, const AmaRecipe **);
AmaStatus ama_object_cleanup(AmaWorld *, AmaHandle root, AmaStatus primary);
AmaStatus ama_object_preflight(AmaObjectOperation *, AmaHandle, bool copy, bool continuation);
void ama_object_unprotect(AmaObjectOperation *);
AmaStatus ama_object_build(AmaObjectOperation *, uint64_t, const AmaRecipe *, AmaHandle *);
AmaStatus ama_object_clone_prepare(AmaObjectOperation *, AmaHandle, AmaHandle final_root,
    bool continuation, void *root_payload, AmaHandle *);
#endif
