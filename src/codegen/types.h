#ifndef AMA_CODEGEN_TYPES_H
#define AMA_CODEGEN_TYPES_H

#include "ast/ast.h"

typedef struct {
    const char* module_path; /* Normalized UTF-8 identity. */
    const AstModule* module; /* Required for generated types; borrowed. */
    const char* native_header; /* Native layout/descriptor provider, or NULL. */
} TypeModule;

/* Emit one header and one implementation with canonical external descriptors.
    Native headers provide the hash-named state typedef and descriptor. Validate
    the entire batch, including inline ownership cycles, before writing either
    stream. Streams must be distinct. Owned scalars are supported; reference/list
    layouts and initializers crossing state/frame boundaries are rejected.
*/
bool emit_type_layouts(FILE* header, FILE* implementation, const char* header_name,
    const TypeModule* modules, size_t module_count, Error* error);

#endif
