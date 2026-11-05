#ifndef AMA_CODEGEN_NATIVE_H
#define AMA_CODEGEN_NATIVE_H

#include "codegen/emitter.h"

bool native_function_name(const char* module_path, char* name, size_t capacity, Error* error);
bool validate_native_signature(SymbolTable* symbols, const char* module_path, Error* error);
bool emit_native_header(FILE* output, SymbolTable* symbols, const char* module_path, Error* error);
bool emit_native_stub(FILE* output, SymbolTable* symbols, const char* module_path, Error* error);
bool emit_native_wrapper(Emitter* emitter, const char* module_path, Error* error);

#endif
