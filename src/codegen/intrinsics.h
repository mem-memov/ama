#ifndef AMA_CODEGEN_INTRINSICS_H
#define AMA_CODEGEN_INTRINSICS_H

#include <stdio.h>
#include "symbols/table.h"

bool register_intrinsics(SymbolTable* symbols, Error* error);
int is_intrinsic_path(const char* path);
bool backend_emit_intrinsics(FILE* out, SymbolTable* symbols, Error* error);

#endif
