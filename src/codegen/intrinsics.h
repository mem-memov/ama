#ifndef AMA_CODEGEN_INTRINSICS_H
#define AMA_CODEGEN_INTRINSICS_H

#include <stdio.h>

void register_intrinsics(void);
int is_intrinsic_path(const char* path);
void backend_emit_intrinsics(FILE* out);

#endif
