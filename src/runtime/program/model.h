/**
 * @file
 * @brief Describe resolved accesses, initializers and explicit native value protocols.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * [REF-01](../../../spec/language/values-and-references.md#ref-01)
 * distinguishes current identity from field selection: self has no payload access tree.
 * All descriptors are borrowed immutable generated data; condition/index callbacks do not imply universal truth or integer conversion.
 * [FLOW-05](../../../spec/language/lists.md#flow-05) Resolved binding descriptors and scope preorder retain lexical identity independently of user spelling.
 */
#ifndef AMA_RUNTIME_PROGRAM_MODEL_H
#define AMA_RUNTIME_PROGRAM_MODEL_H

#include "runtime/objects/objects.h"
struct AmaProgramForeachPlan;
struct AmaProgramForeachScope;

/** Resolved source category; shared immutable storage and a standalone current-instance reference are distinct from field access and literal bytes. */
typedef enum { AMA_SOURCE_LOCAL, AMA_SOURCE_SHARED, AMA_SOURCE_LITERAL, AMA_SOURCE_SELF } AmaProgramSourceKind;
/** Borrowed resolved field selection and optional nested index access. No source-name lookup or heuristic binding occurs at runtime. */
typedef struct AmaProgramAccess {
    size_t field;
    const struct AmaProgramAccess *index;
    bool shared;
    /** Optional compiler-resolved local/reused foreach binding location. */
    const struct AmaProgramForeachPlan *binding;
} AmaProgramAccess;
/** Borrowed resolved initializer/source descriptor with explicit acquisition and continuation intent; literal view length includes embedded zeros. */
typedef struct {
    AmaProgramSourceKind kind;
    const AmaProgramAccess *access;
    size_t shared;
    uint64_t type_id;
    AmaByteView literal;
    bool acquire;
    bool continuation;
} AmaProgramSource;
/** Declared field role drives entry preparation order independently of physical C layout. */
typedef enum { AMA_PROGRAM_PARAMETER, AMA_PROGRAM_RESULT, AMA_PROGRAM_STATE } AmaProgramRole;
/** Borrowed ordered field declaration, selected initializers and exact diagnostic operation; position denotes interface ordering. */
typedef struct {
    AmaProgramRole role;
    size_t position;
    const AmaProgramSource *initializers;
    size_t initializer_count;
    uint64_t operation;
} AmaProgramField;
/** Nominal runtime descriptor plus explicit condition/index protocols; no implicit conversion exists if a protocol is absent. */
typedef struct {
    AmaObjectType *object;
    uint64_t operation;
    const AmaProgramField *fields;
    size_t field_count;
    AmaStatus (*condition)(const void *, bool *);
    AmaStatus (*index)(const void *, size_t *);
    /** Immutable lexical scope preorder; active cells belong to each instance. */
    const struct AmaProgramForeachScope *foreach_scopes;
    size_t foreach_count;
} AmaProgramType;
#endif
