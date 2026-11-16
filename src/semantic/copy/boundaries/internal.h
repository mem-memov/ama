/** @file
 * @brief Retain positive reference-origin proofs within one effect-free lexical statement segment.
 * [REF-07](../../../../spec/language/values-and-references.md#ref-07) makes
 * scalar handle formation/copy nonacquiring; [COPY-06](../../../../spec/language/values-and-references.md#copy-06)
 * requires compile-time rejection when those origins prove forbidden identity or ownership overlap.
 */
#ifndef AMA_SEMANTIC_COPY_BOUNDARIES_INTERNAL_H
#define AMA_SEMANTIC_COPY_BOUNDARIES_INTERNAL_H
#include "semantic/internal.h"
/** A positive must-origin, borrowing a direct owned field or the current instance and its original formation site. Component contract: [REF-07](../../../../spec/language/values-and-references.md#ref-07), [COPY-06](../../../../spec/language/values-and-references.md#copy-06). */
typedef struct ReferenceOrigin {
    struct ReferenceOrigin *next;
    SemanticSymbol *reference;
    SemanticSymbol *owned;
    bool current_instance;
    SourceSpan site;
} ReferenceOrigin;
/** Find exact scalar-reference cell identity in this segment; null means no positive proof. Component contract: [REF-07](../../../../spec/language/values-and-references.md#ref-07), [COPY-06](../../../../spec/language/values-and-references.md#copy-06). */
ReferenceOrigin *semantic_reference_origin(ReferenceOrigin *, const SemanticSymbol *);
/** Update one cell from a fully typed nonindexed scalar handle store; false records allocation failure. Component contract: [REF-07](../../../../spec/language/values-and-references.md#ref-07), [COPY-06](../../../../spec/language/values-and-references.md#copy-06). */
bool semantic_record_origin(SemanticProgram *, ReferenceOrigin **, const SemanticStatement *);
/** Reject only a positively proved continuation identity/ancestor violation; unknown origins remain runtime-checked. Component contract: [REF-07](../../../../spec/language/values-and-references.md#ref-07), [COPY-06](../../../../spec/language/values-and-references.md#copy-06). */
bool semantic_origin_boundary(SemanticProgram *, SemanticType *, ReferenceOrigin *, const SemanticStatement *);
#endif
