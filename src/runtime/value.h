#ifndef AMA_RUNTIME_VALUE_H
#define AMA_RUNTIME_VALUE_H

#include <stddef.h>
#include <stdint.h>

#define AMA_VALUE_ABI_VERSION 1u

typedef enum {
    AMA_STATUS_OK,
    AMA_STATUS_INVALID,
    AMA_STATUS_ABI_MISMATCH,
    AMA_STATUS_TYPE_MISMATCH,
    AMA_STATUS_MEMORY,
    AMA_STATUS_COPY_UNSUPPORTED,
    AMA_STATUS_LITERAL_UNSUPPORTED,
    AMA_STATUS_LITERAL_INVALID
} AmaStatus;

typedef struct AmaType AmaType;

/* Descriptors are immutable, canonical, and live longer than their values.
    Generated descriptors use source_identity_hash(module_path, NULL).
    Pointer identity, not a hash alone, authorizes a layout-compatible copy.
*/
struct AmaType {
    unsigned int abi_version;
    uint64_t identity;
    const char* module_path;
    size_t size;
    size_t alignment;
    const void* context;
    AmaStatus (*initialize)(const AmaType* type, void* destination);
    AmaStatus (*initialize_literal)(const AmaType* type, void* destination,
        const unsigned char* bytes, size_t length);
    AmaStatus (*initialize_copy)(const AmaType* type, void* destination, const void* source);
    void (*move)(const AmaType* type, void* destination, void* source);
    void (*destroy)(const AmaType* type, void* state);
};

/* A borrowed view, not an owning handle or an Ama reference. The caller supplies
    nonoverlapping, correctly aligned storage and tracks whether it is live.
    Initialization callbacks receive uninitialized storage; failure must release
    their partial resources and leave it uninitialized. Move cannot fail and
    leaves its source uninitialized. Destroy cannot fail and ends the lifetime.
    No callback may retain temporary storage addresses. Internal pointers into
    the same state must be repaired by move, rather than blindly copied.
*/
typedef struct {
    const AmaType* type;
    void* state;
} AmaValue;

AmaStatus ama_type_validate(const AmaType* type);
AmaStatus ama_value_initialize(AmaValue destination);
AmaStatus ama_value_initialize_literal(AmaValue destination, const unsigned char* bytes, size_t length);
AmaStatus ama_value_initialize_copy(AmaValue destination, AmaValue source);
AmaStatus ama_value_move(AmaValue destination, AmaValue source);
void ama_value_destroy(AmaValue value);

/* Replacement requires a live destination. Prepare in temporary storage first;
    failure preserves the old value. Commit destroys the old value then moves
    the prepared value without further fallible operations. Self-copy is a no-op.
*/
AmaStatus ama_value_replace(AmaValue destination, AmaValue source);
AmaStatus ama_value_replace_literal(AmaValue destination, const unsigned char* bytes, size_t length);
AmaStatus ama_value_reset(AmaValue destination);

#endif
