/**
 * @file
 * @brief Declare private clone phases under an operation-held source protection.
 *
 * [COPY-07](../../../../spec/language/values-and-references.md#copy-07)
 * requires the complete source tree available before any native clone hook.
 * [COPY-05](../../../../spec/language/values-and-references.md#copy-05)
 * requires final identities and one exact mapping per copy boundary.
 */
#ifndef AMA_OBJECT_CLONE_INTERNAL_H
#define AMA_OBJECT_CLONE_INTERNAL_H
#include "runtime/objects/internal.h"
/** Borrow one complete selected ownership-boundary map during synchronous fixups. Component contract: [COPY-07](../../../../spec/language/values-and-references.md#copy-07), [COPY-05](../../../../spec/language/values-and-references.md#copy-05). */
typedef struct {
    const AmaObjectOperation *operation;
    const AmaObjectWork *boundary;
    AmaHandle final_root;
} AmaCloneMap;
/** Map one full reference identity; external/empty handles remain unchanged. Component contract: [COPY-07](../../../../spec/language/values-and-references.md#copy-07), [COPY-05](../../../../spec/language/values-and-references.md#copy-05). */
AmaHandle ama_clone_reference_map(const void *, AmaHandle);
/** Copy continuation controls only when requested, then remap their reference
 * values with the same exact map as declared fields. No native body is executed.
 * Component contract: [COPY-07](../../../../spec/language/values-and-references.md#copy-07), [COPY-05](../../../../spec/language/values-and-references.md#copy-05). */
AmaStatus ama_clone_control_map(AmaObjectOperation *, AmaObjectWork *, const AmaCloneMap *, bool);

/** Reserve under a previously checked/protected source; parent and root_payload
 * are borrowed. A nonnull returned work owns a reserved slot even on field failure.
 * Arena and pool failure before registration do not change the caller's output.
 * Component contract: [COPY-07](../../../../spec/language/values-and-references.md#copy-07), [COPY-05](../../../../spec/language/values-and-references.md#copy-05). */
AmaStatus ama_clone_reserve(AmaObjectOperation *, AmaHandle source, AmaObjectWork *parent,
    void *root_payload, AmaHandle final_identity, AmaObjectWork **);
/** Descend one protected owned edge; install any reserved child in its parent and
 * update cursor even on a later failure, allowing exact child-first unwinding.
 * Component contract: [COPY-07](../../../../spec/language/values-and-references.md#copy-07), [COPY-05](../../../../spec/language/values-and-references.md#copy-05). */
AmaStatus ama_clone_descend(AmaObjectOperation *, AmaObjectWork **cursor);
#endif
