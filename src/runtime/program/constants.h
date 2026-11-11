#ifndef AMA_RUNTIME_PROGRAM_CONSTANTS_H
#define AMA_RUNTIME_PROGRAM_CONSTANTS_H
#include "model.h"

typedef struct AmaConstantValue AmaConstantValue;
typedef struct { AmaConstantValue *items; size_t count; } AmaConstantField;
/* Shared values are outside every instance pool: no handle, generation,
 * acquisition, return connection or execution position belongs to this storage. */
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
