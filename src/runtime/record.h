#ifndef AMA_RUNTIME_RECORD_H
#define AMA_RUNTIME_RECORD_H

#include "runtime/value.h"

typedef struct {
    size_t offset;
    const AmaType* type;
} AmaField;

typedef struct {
    const AmaField* fields;
    size_t field_count;
} AmaRecord;

/* Generated record descriptors use these adapters and store an immutable
    AmaRecord in context. Fields describe owned values in declaration order.
    Use offsetof on generated C structures; do not calculate C padding manually.
    Layouts must be acyclic. Reference/list adapters are separate future types.
*/
AmaStatus ama_record_initialize(const AmaType* type, void* destination);
AmaStatus ama_record_initialize_copy(const AmaType* type, void* destination, const void* source);
void ama_record_move(const AmaType* type, void* destination, void* source);
void ama_record_destroy(const AmaType* type, void* state);

#endif
