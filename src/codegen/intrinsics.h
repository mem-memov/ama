#ifndef AMA_CODEGEN_INTRINSICS_H
#define AMA_CODEGEN_INTRINSICS_H

#include <stdio.h>
#include "symbols/table.h"

bool register_intrinsics(SymbolTable* symbols, Error* error);
int is_intrinsic_module_path(const char* path);
bool emit_intrinsic_modules(FILE* output, SymbolTable* symbols, Error* error);

#endif
