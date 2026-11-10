#ifndef AMA_CODEGEN_TYPED_NATIVE_H
#define AMA_CODEGEN_TYPED_NATIVE_H

#include "codegen/type_plan.h"

/* Emit prototypes and inline call adapters only after all frame types exist. */
void emit_typed_native_bindings(FILE* output, const PlannedType* type);

#endif
