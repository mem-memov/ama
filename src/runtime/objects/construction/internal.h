/**
 * @file
 * @brief Declare sibling construction phases and their private-work obligations.
 *
 * [LIFE-01](../../../../spec/implementation/native-interface.md#life-01) requires
 * complete children before a payload. [LIFE-03](../../../../spec/implementation/native-interface.md#life-03)
 * requires cleanup of initialized prefixes without destroying failed constructors.
 */
#ifndef AMA_OBJECT_CONSTRUCTION_INTERNAL_H
#define AMA_OBJECT_CONSTRUCTION_INTERNAL_H
#include "runtime/objects/internal.h"

/** Reserve one private slot; expose output only after registration for cleanup.
 * Recipe/source/type records outlive construction. World operation guard is held.
 * Component contract: [LIFE-01](../../../../spec/implementation/native-interface.md#life-01), [LIFE-03](../../../../spec/implementation/native-interface.md#life-03). */
AmaStatus ama_construction_reserve(AmaObjectOperation *, uint64_t type, const AmaRecipe *,
    AmaHandle source, AmaObjectWork *parent, AmaObjectWork **);
/** Resolve one declared/default field and reserve its checked handle storage.
 * No child or payload constructor is invoked here; partial buffers belong to work.
 * Component contract: [LIFE-01](../../../../spec/implementation/native-interface.md#life-01), [LIFE-03](../../../../spec/implementation/native-interface.md#life-03). */
AmaStatus ama_construction_prepare_field(AmaObjectOperation *, AmaObjectWork *);
/** Prepare current field progress; skipped means the driver must revisit this work. Component contract: [LIFE-01](../../../../spec/implementation/native-interface.md#life-01), [LIFE-03](../../../../spec/implementation/native-interface.md#life-03). */
AmaStatus ama_construction_prepare_next(AmaObjectOperation *, AmaObjectWork *, bool *skipped);
/** Reserve the next known owned child; update cursor/location if reservation succeeded,
 * including a later field-initialization failure requiring child-first unwinding.
 * Component contract: [LIFE-01](../../../../spec/implementation/native-interface.md#life-01), [LIFE-03](../../../../spec/implementation/native-interface.md#life-03). */
AmaStatus ama_construction_descend(AmaObjectOperation *, AmaObjectWork **cursor);
/** Invoke exactly the selected payload hook; require all owned children complete. Component contract: [LIFE-01](../../../../spec/implementation/native-interface.md#life-01), [LIFE-03](../../../../spec/implementation/native-interface.md#life-03). */
AmaStatus ama_construction_payload(AmaObjectOperation *, AmaObjectWork *);
/** Publish complete/fixed work records; caller publishes no partially built tree. Component contract: [LIFE-01](../../../../spec/implementation/native-interface.md#life-01), [LIFE-03](../../../../spec/implementation/native-interface.md#life-03). */
void ama_construction_publish(AmaObjectOperation *);
/** Report primary once, clean the current work and its ancestry, preserve primary. Component contract: [LIFE-01](../../../../spec/implementation/native-interface.md#life-01), [LIFE-03](../../../../spec/implementation/native-interface.md#life-03). */
AmaStatus ama_construction_unwind(AmaObjectOperation *, AmaObjectWork *, AmaStatus primary);
#endif
