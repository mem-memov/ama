/**
 * @file
 * @brief Represent shared immutable values outside instance pools.
 *
 * [CONST-01](../../../spec/language/declarations.md#const-01)
 * Constant storage has no handle, acquisition or saved execution position; parent/cursors exist only for iterative lifecycle cleanup.
 */
#ifndef AMA_RUNTIME_PROGRAM_CONSTANTS_H
#define AMA_RUNTIME_PROGRAM_CONSTANTS_H
#include "model.h"

/**
 * Forward name for immutable shared storage outside pools; the complete owned arrays and
 * partial-cleanup ledger follow.
 */
typedef struct AmaConstantValue AmaConstantValue;
/** Owned array of immutable shared child values; count also bounds partial reverse cleanup. */
typedef struct { AmaConstantValue *items; size_t count; } AmaConstantField;
/* Shared values are outside every instance pool: no handle, generation,
 * acquisition, return connection or execution position belongs to this storage. */
/** Shared storage outside pools. Own fields/allocation, borrow type/parent; native_live and remaining_fields provide allocation-free partial cleanup. */
struct AmaConstantValue {
    const AmaProgramType *type;
    AmaConstantField *fields;
    void *allocation;
    void *payload;
    bool native_live;
    struct AmaConstantValue *parent;
    size_t remaining_fields;
};
#endif
